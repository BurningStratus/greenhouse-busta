#include "buttons.hpp"

#include <cmath>

#include "hardware/gpio.h"
#include "FreeRTOS.h"

#define BUTTON_PIN_1   9
#define BUTTON_PIN_2   8
#define BUTTON_PIN_3   7
#define BUTTON_PIN_MAX 9

static QueueHandle_t *buttonsQueue;

static void gpio_callback(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Check if we received an irq concerning a button and send a message
    if (gpio == BUTTON_PIN_1 || gpio == BUTTON_PIN_2 || gpio == BUTTON_PIN_3) {
        const int button = abs(static_cast<int>(gpio) - BUTTON_PIN_MAX); // abs(gpio - max) = [0-2]
        xQueueSendFromISR(*buttonsQueue, &button, &xHigherPriorityTaskWoken);
    }

    // Announce if we woke a higher priority task for context switch
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void initButtons() {
    // Initialize buttons
    gpio_init(BUTTON_PIN_1);
    gpio_set_dir(BUTTON_PIN_1, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_1);

    gpio_init(BUTTON_PIN_2);
    gpio_set_dir(BUTTON_PIN_2, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_2);

    gpio_init(BUTTON_PIN_3);
    gpio_set_dir(BUTTON_PIN_3, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_3);

    // Set an irq for the buttons
    gpio_set_irq_enabled_with_callback(BUTTON_PIN_1, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    gpio_set_irq_enabled(BUTTON_PIN_2, GPIO_IRQ_EDGE_RISE, true);
    gpio_set_irq_enabled(BUTTON_PIN_3, GPIO_IRQ_EDGE_RISE, true);
}

void linkUserInterfaceQueueToGpioCallback(QueueHandle_t &userInterfaceQueue) {
    buttonsQueue = &userInterfaceQueue;
}