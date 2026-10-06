#include "UserInterfaceTask.hpp"

#include "buttons/buttons.hpp"
#include "debug/Debug.hpp"

namespace {
    // Possible screens for interface
    enum class UiScreen {
        SENSORS,
        CONFIG,
        SIZE
    };
}

void userInterfaceTask(void *pvParameters)
{
    const auto *params = static_cast<UserInterfaceParams *>(pvParameters);

    // Different views available
    SensorView sensorView;
    ParamsView paramsView;

    // Current screen
    auto screen = UiScreen::SENSORS;

    // Data received
    SensorData data{};
    Button button;

    // Config
    ConfigStorage& cfg = ConfigStorage::instance();

    while (true) {
        bool redraw = false;

        // Check buttons
        if (xQueueReceive(params->buttonQueue, &button, 0) == pdTRUE) {
            if (button == Button::SW0) {
                screen = static_cast<UiScreen>((static_cast<int>(screen) + 1) % static_cast<int>(UiScreen::SIZE));
                redraw = true;
            } else if (screen == UiScreen::CONFIG && button == Button::SW1) {
                cfg.global_config().co2Target -= 10;
                // TODO not store every time we change it ?
                if (!cfg.config_store()) {
                    debug(xTaskGetTickCount(),
                          &params->debugQueue,
                          "Could not store config from userInterfaceTask\n", 0, 0, 0);
                }
                redraw = true;
            } else if (screen == UiScreen::CONFIG && button == Button::SW2) {
                cfg.global_config().co2Target += 10;
                if (!cfg.config_store()) {
                    debug(xTaskGetTickCount(),
                          &params->debugQueue,
                          "Could not store config from userInterfaceTask\n", 0, 0, 0);
                }
                redraw = true;
            }
        }

        // Get newest sensor values
        if (xQueueReceive(params->sensorQueue, &data, 0) == pdTRUE) {
            if (screen == UiScreen::SENSORS) {
                redraw = true;
            }
        }

        // Redraw screen if needed
        if (redraw) {
            params->display.fill(0);

            if (screen == UiScreen::SENSORS) {
                sensorView.generate(data);
                params->display.blit(sensorView, 0, 0);
            } else if (screen == UiScreen::CONFIG) {
                paramsView.generate(cfg.global_config());
                params->display.blit(paramsView, 0, 0);
            } else {
                debug(xTaskGetTickCount(),
                      &params->debugQueue,
                      "Trying to redraw an unknown screen from userInterfaceTask()\n", 0, 0, 0);
            }

            params->display.show();
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}