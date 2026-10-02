#pragma once

#include <cstdint>

#include "FreeRTOS.h"
#include "queue.h"

/**
 * @brief Defines a DebugEvent with a timestamp, a format for printf() and three parameters to give the function
 *        for use in the debugTask() and debug() functions
 */
struct DebugEvent {
    const TickType_t  timestamp;
    const char       *format;
    uint32_t          data[3];
};

/**
 * @brief Stores parameters for debugTask() function
 */
struct DebugParams {
    const QueueHandle_t *debugQueue;
};

void debug(TickType_t timestamp, const QueueHandle_t *debugQueue, const char *format,
            uint32_t d1, uint32_t d2, uint32_t d3);

void debugTask(void *pvParameters);