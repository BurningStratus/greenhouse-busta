#pragma once

#include <cstddef>
#include <cstdint>

#include "hardware/i2c.h"
#include "FreeRTOS.h"
#include "semphr.h"

class I2c {
public:
    // Constructor
    I2c(i2c_inst_t *i2cInstance, uint8_t sdaPin, uint8_t sclPin, uint32_t baudRate);

    // Destructor
    ~I2c();

    // Copy
    I2c(const I2c &) = delete;
    I2c &operator=(const I2c &) = delete;

    // Methods
    int write(uint8_t address, const uint8_t *data, size_t length);
    int read(uint8_t address, uint8_t *data, size_t length);
    int writeRead(uint8_t address, const uint8_t *writeData, size_t writeLength, uint8_t *readData, size_t readLength);

private:
    // Attributes
    i2c_inst_t *i2cInstance;
    SemaphoreHandle_t mutex;
};