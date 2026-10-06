#include "buttons.hpp"

#include "hardware/gpio.h"

#define BUTTON_PIN_1 9
#define BUTTON_PIN_2 8
#define BUTTON_PIN_3 7

static QueueHandle_t buttonsQueue = nullptr;

static void gpio_callback(uint gpio, uint32_t events)
{
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    Button button;

    if (gpio == BUTTON_PIN_1) {
        button = Button::SW0;
    } else if (gpio == BUTTON_PIN_2) {
        button = Button::SW1;
    } else if (gpio == BUTTON_PIN_3) {
        button = Button::SW2;
    } else {
        return;
    }

    if (buttonsQueue != nullptr) {
        xQueueSendFromISR(buttonsQueue, &button, &higherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void linkButtonQueueToGpioCallback(QueueHandle_t queue)
{
    buttonsQueue = queue;
}

void initButton()
{
    constexpr uint pins[] = { BUTTON_PIN_1, BUTTON_PIN_2, BUTTON_PIN_3 };

    // enable each button
    for (const uint pin : pins) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);
    }

    // set ISR for each button pin
    gpio_set_irq_enabled_with_callback(BUTTON_PIN_1, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(BUTTON_PIN_2, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BUTTON_PIN_3, GPIO_IRQ_EDGE_FALL, true);
}