#include <cstdio>
#include "FreeRTOS.h"
#include "task.h"
#include "pico/stdio.h"
#include "hardware/gpio.h"
#include "shared/SensorData.h"
#include "control/FanValveTask.h"
#include "pico/platform/panic.h"
#include "Uart/PicoOsUart.h"
#include "modbus/Modbus.h"
#include "actuators/Fan.h"
#include "shared/ControlStatus.h"
#include "shared/ControlConfig.h"

// needed for runtime statistics
#include <cstring>

#include "queue.h"
#include "hardware/timer.h"

#include <iostream> // I/O streams for printing

// Read the board if you want to check
#define UART_NR         1
#define UART_TX_PIN     4
#define UART_RX_PIN     5
#define UART_SPEED   9600
#define UART_STOP_NR    2

// Modbus addresses
#define CO2_MODBUS_ADDRESS      240 // GMP252
#define HUMIDITY_MODBUS_ADDRESS 241 // HMP60

// Modbus functions/registers
#define MODBUS_READ_HOLDING_REGISTERS         0x03
#define CO2_MODBUS_READ_REGISTER_ADDRESS      0x0000
#define HUMIDITY_MODBUS_READ_REGISTER_ADDRESS 0x0000

extern "C" {
uint32_t read_runtime_ctr(void) {
    return timer_hw->timerawl;
}
}

// stack overflow check
extern "C" {
void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName ) {
    if (pcTaskName != nullptr) panic("Stack overflow: %s",pcTaskName);
    else panic("Stack overflow of unnamed task");
}
}

struct DebugEvent {
    const TickType_t timestamp;
    const char *format;
    uint32_t data[3];
};

struct DebugParams {
    const QueueHandle_t *debugQueue;
};

static void debug(const TickType_t timestamp, const QueueHandle_t *debugQueue, const char *format,
                  const uint32_t d1, const uint32_t d2, const uint32_t d3)
{
    const DebugEvent event{timestamp, format, d1, d2, d3};
    xQueueSend(*debugQueue, &event, portMAX_DELAY);
}

static void debugTask(void *pvParameters)
{
    const auto *debugParams = static_cast<DebugParams *>(pvParameters);
    char buffer[64];
    uint8_t spaceLeft = sizeof(buffer);
    DebugEvent e{};

    while (true) {
        // read queue
        xQueueReceive(*(debugParams->debugQueue), &e, portMAX_DELAY);

        // build message and print
        spaceLeft -= snprintf(buffer, sizeof(buffer), "%lu - ", e.timestamp);
        snprintf(buffer, spaceLeft, e.format, e.data[0], e.data[1], e.data[2]);
        std::cout << buffer;
    }
}

struct SensorParams {
    Modbus *modbus;
    QueueHandle_t *sensorQueue;
    QueueHandle_t *debugQueue;
};

static void sensorReader(void *pvParameters) {
    const auto *sensorParams = static_cast<SensorParams*>(pvParameters);
    uint16_t co2Regs[2], humidRegs[2];
    uint32_t co2Raw, humidRaw;
    bool success;

    while (true) {
        // Retrieve current time
        TickType_t now = xTaskGetTickCount();

        // Initialize data
        // TODO add pressure reading (right now it's set to 0)
        read_data co2{}, humid{}, press{};

        // Read co2
        success = sensorParams->modbus->readRegisters(CO2_MODBUS_ADDRESS,
            MODBUS_READ_HOLDING_REGISTERS, CO2_MODBUS_READ_REGISTER_ADDRESS, co2Regs, 2);
        if (!success) {
            debug(xTaskGetTickCount(), sensorParams->debugQueue,
                  "CO2 Modbus read failed\n", 0, 0, 0);
        }else
        {
            // Convert co2 to float (we receive it reversed)
            co2Raw = (static_cast<uint32_t>(co2Regs[1]) << 16) | static_cast<uint32_t>(co2Regs[0]);
            static_assert(sizeof(co2.value) == sizeof(co2Raw));
            memcpy(&co2.value, &co2Raw, sizeof(co2.value));
            co2.valid = true;
        }

        // debug the received value
        // TODO remove
        printf("CO2: %f\n", co2.value);

        // Read humidity
        success = sensorParams->modbus->readRegisters(HUMIDITY_MODBUS_ADDRESS,
            MODBUS_READ_HOLDING_REGISTERS, HUMIDITY_MODBUS_READ_REGISTER_ADDRESS, humidRegs, 2);
        if (!success) {
            debug(xTaskGetTickCount(), sensorParams->debugQueue,
                  "CO2 Modbus read failed\n", 0, 0, 0);
        }

        // Convert humidity to float (we receive it reversed)
        humidRaw = (static_cast<uint32_t>(humidRegs[1]) << 16) | static_cast<uint32_t>(humidRegs[0]);
        static_assert(sizeof(humid.value) == sizeof(humidRaw));
        memcpy(&humid.value, &humidRaw, sizeof(humid.value));

        // debug the received value
        // TODO remove
        printf("Humidity: %f\n", humid.value);

        // Send data to queue
        SensorData sensorData{co2, humid, {}, press, now};
        xQueueSend(*sensorParams->sensorQueue, &sensorData, pdMS_TO_TICKS(10));

        // Wait until next wake up
        // TODO check if we cannot do it periodically instead of waiting which causes jitter in the long run
        vTaskDelay(pdMS_TO_TICKS(1000)); // every 1s
    }
}

int main()
{
    // create config
    const TickType_t maxAge = pdMS_TO_TICKS(5000);
    const bool valve_open_level = true;
    
    // Create debug queue for debugging messages
    QueueHandle_t debugQueue = xQueueCreate(10, sizeof(DebugEvent));

    // Create debug parameters
    DebugParams debugParams{.debugQueue = &debugQueue};

    // Create sensor queue for sharing sensor data
    QueueHandle_t sensorQueue = xQueueCreate(1, sizeof(SensorData));
    if (sensorQueue == nullptr) {
        panic("could not create sensor data queue");
    }

    // Keep the latest settings, the UI can replace them with xQueueOverwrite
    QueueHandle_t configQueue = xQueueCreate(1, sizeof(ControlConfig));
    if (configQueue == nullptr)
    {
        panic("could not create control config queue");
    }
    const ControlConfig initialConfig{};
    xQueueOverwrite(configQueue, &initialConfig);

    // Keep the latest control status
    QueueHandle_t statusQueue = xQueueCreate(1, sizeof(ControlStatus));
    if (statusQueue == nullptr)
    {
        panic("could not create control status queue");
    }

    // create modbus and fan
    PicoOsUart modbusUart{UART_NR, UART_TX_PIN, UART_RX_PIN, UART_SPEED, UART_STOP_NR};
    Modbus modbus{&modbusUart};

    // Create parameters for sensor task
    SensorParams sensorParams{.modbus = &modbus, .sensorQueue = &sensorQueue, .debugQueue = &debugQueue};

    // Initialization of valve and fan
    Valve valve{27,valve_open_level};
    Fan fan{&modbus};
    FanValveTaskParams fanValveParams{sensorQueue, &valve, configQueue, maxAge, &fan, statusQueue};
    
    stdio_init_all();

    printf("\nBoot\n");

    // create all tasks and start scheduler
    xTaskCreate(debugTask, "debug", 512, (void *)&debugParams, tskIDLE_PRIORITY + 1, nullptr);
    xTaskCreate(sensorReader, "sensorReader", 512, (void *)&sensorParams, tskIDLE_PRIORITY + 2, nullptr);
    BaseType_t i = xTaskCreate(FanValveTask, "fanValve", 512, &fanValveParams, tskIDLE_PRIORITY + 2, nullptr);
    if (i != pdPASS) {
        panic("could not create fan valve task");
    }

    vTaskStartScheduler();

    while(true){};
}
