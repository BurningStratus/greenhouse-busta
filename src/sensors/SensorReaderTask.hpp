#pragma once

#include "queue.h"
#include "drivers/Modbus.h"
#include "drivers/I2c.hpp"

/**
 * @brief Stores parameters for sensorReader() function
 */
struct SensorParams {
    Modbus        *modbus;
    I2c           *i2c;
    QueueHandle_t *controlSensorQueue;
    QueueHandle_t *uiSensorQueue;
    QueueHandle_t *debugQueue;
};

void sensorReader(void *pvParameters);