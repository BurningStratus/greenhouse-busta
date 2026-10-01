/**
 *  @file       main.cpp
 *  @authors    Fabien Léger, Pere Joan Garriga Voltas and Pavel Shishkin
 *  @version    0.1
 *  @date       30.09.2026
 *  @link       https://github.com/BurningStratus/greenhouse-busta
 *
 *  @brief      main for the Greenhouse Busta project where we open a fan/valve depending on sensors from the outside
 *              and inside of a chamber to maintain co2 levels inside it to a chosen level
 */

// General c, c++ and FreeRTOS libraries
#include <cstdio>
#include <iostream> // I/O streams for printing
#include <cstring> // Needed for runtime statistics
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "hardware/timer.h"

// Pico related libraries
#include "pico/stdio.h"
#include "hardware/gpio.h"
#include "pico/platform/panic.h"
#include "hardware/i2c.h"

// Self-made drivers
#include "drivers/PicoOsUart.hpp"
#include "drivers/I2c.hpp"

// pere-modbus-control for handling fan, valve and modbus communication
#include "actuators/Fan.h"
#include "control/FanValveTask.h"
#include "drivers/Modbus.h"
#include "shared/ControlConfig.h"
#include "shared/ControlStatus.h"
#include "shared/SensorData.h"

// Sensors
#include "sensors/Sdp6xx.hpp"

// Read the board if you want to check
#define UART_NR         1
#define UART_TX_PIN     4
#define UART_RX_PIN     5
#define UART_SPEED   9600
#define UART_STOP_NR    2

// Modbus addresses
#define CO2_MODBUS_ADDRESS  240 // GMP252
#define HUM_MODBUS_ADDRESS  241 // HMP60
#define TEMP_MODBUS_ADDRESS 240 // GMP252

// Modbus functions/registers
#define MODBUS_READ_HOLDING_REGISTERS       0x03
#define CO2_MODBUS_READ_REGISTER_ADDRESS  0x0000
#define HUM_MODBUS_READ_REGISTER_ADDRESS  0x0000
#define TEMP_MODBUS_READ_REGISTER_ADDRESS 0x0004

// I2C1 for pressure sensor and OLED
#define I2C1_SDA_PIN       14
#define I2C1_SCL_PIN       15
#define I2C1_BAUD_RATE 100000

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

/**
 * @brief Defines a DebugEvent with a timestamp, a format for printf() and three parameters to give the function
 *        for use in the debugTask() and debug() functions
 */
struct DebugEvent {
    const TickType_t timestamp;
    const char *format;
    uint32_t data[3];
};

/**
 * @brief Stores parameters for debugTask() function
 */
struct DebugParams {
    const QueueHandle_t *debugQueue;
};

/**
 * @brief Simplified function to create a DebugEvent and sending it to a debug queue
 *
 * @param timestamp time the event was recorded, usually with xTaskGetTickCount()
 * @param debugQueue queue to send the debug message to
 * @param format format in the same form as printf()
 * @param d1 first parameter to give to format
 * @param d2 second parameter to give to format
 * @param d3 third parameter to give to format
 */
static void debug(const TickType_t timestamp, const QueueHandle_t *debugQueue, const char *format,
                  const uint32_t d1, const uint32_t d2, const uint32_t d3)
{
    const DebugEvent event{timestamp, format, d1, d2, d3};
    xQueueSend(*debugQueue, &event, portMAX_DELAY);
}

/**
 * @brief Debug task that receive messages from a queue and prints them when program is IDLE
 *
 * @param pvParameters parameters in the form of DebugParams
 */
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

/**
 * @brief Stores parameters for sensorReader() function
 */
struct SensorParams {
    Modbus *modbus;
    Sdp6xx *sdp610;
    QueueHandle_t *sensorQueue;
    QueueHandle_t *debugQueue;
};

/**
 * @brief Read a specific modbus address and register address with a function code given
 *
 * @param sensorParams parameters given to the sensorReader() function
 * @param modbus_address address of the modbus
 * @param function_code function code to read
 * @param register_address address of the register itself inside the device connected through modbus
 *
 * @return read_data being read through modbus
 */
static ReadData readModbusSensor(const SensorParams *sensorParams,
                                  const uint8_t modbus_address,
                                  const uint8_t function_code,
                                  const uint16_t register_address) {
    ReadData data;
    uint16_t regs[2];
    uint32_t raw;
    bool success;

    success = sensorParams->modbus->readRegisters(modbus_address,
                                                  function_code,
                                                  register_address,
                                                  regs,
                                                  2);
    if (!success) {
        debug(xTaskGetTickCount(), sensorParams->debugQueue, "Modbus read failed\n", 0, 0, 0);
    } else {
        // Convert value to float (receiving the registers reversed
        raw = (static_cast<uint32_t>(regs[1]) << 16) | static_cast<uint32_t>(regs[0]);
        static_assert(sizeof(data.value) == sizeof(raw));
        memcpy(&data.value, &raw, sizeof(data.value));
        data.valid = true;
    }

    return data;
}

/**
 * @brief Read different sensors such as co2, humidity, temperature and pressure and send them to a queue
 *
 * @param pvParameters pointer to parameters in the form of SensorParams
 */
static void sensorReader(void *pvParameters) {
    const auto *sensorParams = static_cast<SensorParams*>(pvParameters);

    while (true) {
        // Initialize data
        ReadData co2{}, hum{}, temp{}, press{};

        co2 = readModbusSensor(sensorParams,
                               CO2_MODBUS_ADDRESS,
                               MODBUS_READ_HOLDING_REGISTERS,
                               CO2_MODBUS_READ_REGISTER_ADDRESS);

        hum = readModbusSensor(sensorParams,
                               HUM_MODBUS_ADDRESS,
                               MODBUS_READ_HOLDING_REGISTERS,
                               HUM_MODBUS_READ_REGISTER_ADDRESS);

        temp = readModbusSensor(sensorParams,
                                TEMP_MODBUS_ADDRESS,
                                MODBUS_READ_HOLDING_REGISTERS,
                                TEMP_MODBUS_READ_REGISTER_ADDRESS);

        if (sensorParams->sdp610->readPressure(press.value) < 0) {
            press.valid = false;
        }

        // debug the received values
        // TODO remove debug
        printf("CO2: %f\n", co2.value);
        printf("Humidity: %f\n", hum.value);
        printf("Temperature: %f\n", temp.value);
        printf("Pressure: %f\n", press.value);

        // Send data to queue
        SensorData sensorData{.co2 = co2,
                              .humidity = hum,
                              .temperature = temp,
                              .pressure = press,
                              .timestamp = xTaskGetTickCount()};
        xQueueSend(*sensorParams->sensorQueue, &sensorData, pdMS_TO_TICKS(10));

        // Wait until next wake up
        // TODO check if we cannot do it periodically instead of waiting which causes jitter in the long run
        vTaskDelay(pdMS_TO_TICKS(1000)); // every 1s
    }
}

int main() {
    // create config
    constexpr TickType_t maxAge = pdMS_TO_TICKS(5000);
    constexpr bool valve_open_level = true;
    
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
    if (configQueue == nullptr) {
        panic("could not create control config queue");
    }
    const ControlConfig initialConfig{};
    xQueueOverwrite(configQueue, &initialConfig);

    // Keep the latest control status
    QueueHandle_t statusQueue = xQueueCreate(1, sizeof(ControlStatus));
    if (statusQueue == nullptr) {
        panic("could not create control status queue");
    }

    // Create modbus and fan
    PicoOsUart modbusUart{UART_NR, UART_TX_PIN, UART_RX_PIN, UART_SPEED, UART_STOP_NR};
    Modbus modbus{&modbusUart};

    // Create i2c connection for pressure and OLED
    I2c i2c{i2c1, I2C1_SDA_PIN, I2C1_SCL_PIN, I2C1_BAUD_RATE};
    Sdp6xx sdp610{i2c};

    // Create parameters for sensor task
    SensorParams sensorParams{.modbus = &modbus,
                              .sdp610 = &sdp610,
                              .sensorQueue = &sensorQueue,
                              .debugQueue = &debugQueue};

    // Initialization of valve and fan
    Valve valve{27,valve_open_level};
    Fan fan{&modbus};
    FanValveTaskParams fanValveParams{sensorQueue, &valve, configQueue, maxAge, &fan, statusQueue};
    
    stdio_init_all();

    printf("\nBoot\n");

    // create all tasks and start scheduler
    // TODO why do we check only one task creation?
    xTaskCreate(debugTask, "debug", 512, (void *)&debugParams, tskIDLE_PRIORITY + 1, nullptr);
    xTaskCreate(sensorReader, "sensorReader", 512, (void *)&sensorParams, tskIDLE_PRIORITY + 2, nullptr);
    BaseType_t i = xTaskCreate(FanValveTask, "fanValve", 512, &fanValveParams, tskIDLE_PRIORITY + 2, nullptr);
    if (i != pdPASS) {
        panic("could not create fan valve task");
    }

    vTaskStartScheduler();

    while(true){};
}
