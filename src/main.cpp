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

// needed for runtime statistics
#include "queue.h"
#include "hardware/timer.h"

#include <iostream> // I/O streams for printing

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
    const QueueHandle_t *debug_queue;
};

static void debug(const TickType_t timestamp, const QueueHandle_t *debug_queue, const char *format,
                  const uint32_t d1, const uint32_t d2, const uint32_t d3)
{
    const DebugEvent event{timestamp, format, d1, d2, d3};
    xQueueSend(*debug_queue, &event, portMAX_DELAY);
}

static void debugTask(void *pvParameters)
{
    const auto *debug_params = static_cast<DebugParams *>(pvParameters);
    char buffer[64];
    uint8_t space_left = sizeof(buffer);
    DebugEvent e{};

    while (true) {
        // read queue
        xQueueReceive(*(debug_params->debug_queue), &e, portMAX_DELAY);

        // build message and print
        space_left -= snprintf(buffer, sizeof(buffer), "%lu - ", e.timestamp);
        snprintf(buffer, space_left, e.format, e.data[0], e.data[1], e.data[2]);
        std::cout << buffer;
    }
}

// test task to verify the pico works
static void testTask(void *pvParameters) {
    const auto *debug_params = static_cast<DebugParams *>(pvParameters);
    debug(xTaskGetTickCount(), debug_params->debug_queue, "Message from testTask!%d%d%d\n", 1, 2, 3);

    while (true) {
        vTaskDelay(10000);
    }
}

int main()
{
    // create config
    const float co2_target = 1000.0f;
    const TickType_t max_age = pdMS_TO_TICKS(5000);
    const bool valve_open_level = true;


    // create global variables
    QueueHandle_t debug_queue = xQueueCreate(10, sizeof(DebugEvent));

    // create queue for sensor data
    QueueHandle_t sensor_queue = xQueueCreate(1, sizeof(SensorData));
    if (sensor_queue == nullptr) {
        return -1;
    }


    // package parameters
    DebugParams debug_params{&debug_queue};

    // Uart 1, GPIO transceiver, GPIO receiver
    static PicoOsUart modbus_uart{1, 4, 5, 9600, 2};

    // create modbus and fan
    static Modbus modbus{&modbus_uart};
    static Fan fan{&modbus};

    // Valve and fan initialization
    static Valve valve{27,valve_open_level};
    static FanValveTaskParams fan_valve_params{sensor_queue, &valve, co2_target, max_age, &fan};



    stdio_init_all();

    printf("\nBoot\n");

    // create all tasks and start scheduler
    xTaskCreate(debugTask, "debug", 512, (void *)&debug_params, tskIDLE_PRIORITY + 1, nullptr);
    xTaskCreate(testTask, "test", 512, (void *)&debug_params, tskIDLE_PRIORITY + 2, nullptr);
    BaseType_t i = xTaskCreate(FanValveTask, "fanValve", 512, &fan_valve_params, tskIDLE_PRIORITY + 2, nullptr);
    if (i != pdPASS) {
        panic("could not create fan valve task");
    }

    vTaskStartScheduler();

    while(true){};
}