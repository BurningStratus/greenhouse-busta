#include <cstdio>

#include "FreeRTOS.h"
#include "SensorReaderTask.hpp"
#include "sensors/Gmp252.hpp"
#include "sensors/Hmp60.hpp"
#include "sensors/Sdp6xx.hpp"
#include "shared/SensorData.h"

/**
 * @brief Read different sensors such as co2, humidity, temperature and pressure and send them to a queue
 *
 * @param pvParameters pointer to parameters in the form of SensorParams
 */
void sensorReader(void *pvParameters) {
    const auto *sensorParams = static_cast<SensorParams*>(pvParameters);

    const Gmp252 gmp252(*sensorParams->modbus);
    const Hmp60  hmp60(*sensorParams->modbus);
    const Sdp6xx sdp610{*sensorParams->i2c};

    TickType_t lastWake = xTaskGetTickCount();
    while (true) {
        // Initialize data
        ReadData co2{}, hum{}, temp{}, press{};

        // Read values
        co2.valid   = gmp252.readCo2(co2.value);
        temp.valid  = gmp252.readTemperature(temp.value);
        hum.valid   = hmp60.readHumidity(hum.value);
        press.valid = sdp610.readPressure(press.value);

        // debug the received values
        // TODO remove debug
        printf("CO2: %f\n", co2.value);
        printf("Humidity: %f\n", hum.value);
        printf("Temperature: %f\n", temp.value);
        printf("Pressure: %f\n", press.value);

        // Send data to queue
        SensorData sensorData{.co2         = co2,
                              .humidity    = hum,
                              .temperature = temp,
                              .pressure    = press,
                              .timestamp   = xTaskGetTickCount()};
        xQueueSend(*sensorParams->controlSensorQueue, &sensorData, pdMS_TO_TICKS(10));
        xQueueOverwrite(*sensorParams->uiSensorQueue, &sensorData);

        // Wait until next wake up
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(1000));
    }
}