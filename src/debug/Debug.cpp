#include "Debug.hpp"

#include <cstdio>
#include <iostream>

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
void debug(const TickType_t timestamp, const QueueHandle_t *debugQueue, const char *format,
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
void debugTask(void *pvParameters)
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