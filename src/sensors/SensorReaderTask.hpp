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
    QueueHandle_t *sensorQueue;
    QueueHandle_t *debugQueue;
};

void sensorReader(void *pvParameters);