#pragma once

#include "FreeRTOS.h"
#include "queue.h"

enum class Button {
    SW0,
    SW1,
    SW2
};

void initButton();
void linkUserInterfaceQueueToGpioCallback(QueueHandle_t queue);