#pragma once

#include "drivers/Display.hpp"
#include "queue.h"

struct UserInterfaceParams {
    QueueHandle_t &sensorQueue;
    QueueHandle_t &buttonQueue;
    QueueHandle_t &debugQueue;
    Display &display;
};

void userInterfaceTask(void *pvParameters);