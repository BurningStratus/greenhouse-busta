#include "UserInterfaceTask.hpp"

#include "buttons/buttons.hpp"

// Possible screens for interface
enum class UiScreen {
    Sensors,
    Config
};

void userInterfaceTask(void *pvParameters)
{
    const auto *params = static_cast<UserInterfaceParams *>(pvParameters);

    SensorView sensorView;
    UiScreen screen = UiScreen::Sensors;
    SensorData data{};
    Button button;

    while (true) {
        if (xQueueReceive(params->sensorQueue,
                          &data,
                          portMAX_DELAY) == pdTRUE) {

            sensorView.generate(data);

            params->display.fill(0);
            params->display.blit(sensorView, 0, 0);
            params->display.show();
        }
    }
}