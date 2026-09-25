#include <cstdio>
#include "FreeRTOS.h"
#include "task.h"
#include "pico/stdio.h"
#include "hardware/gpio.h"

// needed for runtime statistics
#include "queue.h"
#include "hardware/timer.h"

#include <iostream> // I/O streams for printing

using namespace std;

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
        cout << buffer;
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
    // create global variables
    QueueHandle_t debug_queue = xQueueCreate(10, sizeof(DebugEvent));

    // package parameters
    DebugParams debug_params{&debug_queue};

    stdio_init_all();

    printf("\nBoot\n");

    // create all tasks and start scheduler
    xTaskCreate(debugTask, "debug", 512, (void *)&debug_params, tskIDLE_PRIORITY + 1, nullptr);
    xTaskCreate(testTask, "test", 512, (void *)&debug_params, tskIDLE_PRIORITY + 2, nullptr);
    vTaskStartScheduler();

    while(true){};
}