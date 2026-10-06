#pragma once

#include "FreeRTOS.h"
#include "queue.h"
#include "pico/platform/panic.h"
#include "hardware/gpio.h"

#include <cmath>

#define BUTTON_PIN_1   9
#define BUTTON_PIN_2   8
#define BUTTON_PIN_3   7
#define BUTTON_PIN_MAX 9
#define INPUT_QUEUE_LENGTH 100

enum class Button {
    SW0,
    SW1,
    SW2
};

static QueueHandle_t *input_queue;

void initButton();
void linkUserInterfaceQueueToGpioCallback(QueueHandle_t &userInterfaceQueue);
static QueueHandle_t *buttonsQueue;

static void gpio_callback (uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Check if we received an irq concerning a button and send a message
    if (gpio == BUTTON_PIN_1 || gpio == BUTTON_PIN_2 || gpio == BUTTON_PIN_3) {
        const int button = abs(static_cast<int>(gpio) - BUTTON_PIN_MAX); // abs(gpio - max) = [0-2]
        xQueueSendFromISR(*buttonsQueue, &button, &xHigherPriorityTaskWoken);
    }

    // Announce if we woke a higher priority task for context switch
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

QueueHandle_t initButton
// (unsigned int gpio, QueueHandle_t evt_queue, void (*fptr)(unsigned int, uint32_t))
(unsigned int gpio, QueueHandle_t evt_queue)
{
    // Initialize buttons
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
    gpio_set_irq_enabled_with_callback (gpio, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    QueueHandle_t queue = xQueueCreate (INPUT_QUEUE_LENGTH, sizeof (Button::SW0));
    if (queue == nullptr)
        panic ("error allocating queue");        

    return queue;
}

