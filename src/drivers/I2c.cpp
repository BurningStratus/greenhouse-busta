#include "I2c.hpp"

#include "hardware/gpio.h"

/**
 * @brief Creates an I2C connection from an I2C instance, pins and baud rate
 *
 * Initializes the selected I2C peripheral, configures the SDA and SCL pins
 * and creates a mutex to protect the bus from concurrent access.
 *
 * @param i2cInstance I2C peripheral instance, for example i2c0 or i2c1
 * @param sdaPin GPIO pin used for SDA
 * @param sclPin GPIO pin used for SCL
 * @param baudRate I2C bus speed in Hz
 */
I2c::I2c(i2c_inst_t *i2cInstance, const uint8_t sdaPin, const uint8_t sclPin, const uint32_t baudRate)
         : i2cInstance(i2cInstance)
{
    // Initialize the I2C peripheral
    i2c_init(i2cInstance, baudRate);

    // Configure the selected GPIO pins for I2C operation
    gpio_set_function(sdaPin, GPIO_FUNC_I2C);
    gpio_set_function(sclPin, GPIO_FUNC_I2C);

    // Enable pull-up resistors required by the I2C bus
    gpio_pull_up(sdaPin);
    gpio_pull_up(sclPin);

    // Create a mutex so only one task can access the I2C bus at a time
    mutex = xSemaphoreCreateMutex();

    if (mutex == nullptr) {
        panic("Failed to create I2C mutex");
    }
}

/**
 * @brief Destroys the I2C connection
 *
 * Deletes the mutex and deinitializes the I2C peripheral.
 */
I2c::~I2c()
{
    if (mutex != nullptr) {
        vSemaphoreDelete(mutex);
    }

    i2c_deinit(i2cInstance);
}

/**
 * @brief Writes data to an I2C device
 *
 * The I2C bus is locked during the transaction to prevent another task
 * from accessing it simultaneously.
 *
 * @param address 7-bit I2C address of the target device
 * @param data Pointer to the data to send
 * @param length Number of bytes to send
 *
 * @return Number of bytes written, or a negative value on error
 */
int I2c::write(const uint8_t address, const uint8_t *data, const size_t length)
{
    // Wait until the I2C bus is available
    xSemaphoreTake(mutex, portMAX_DELAY);

    const int result = i2c_write_blocking(i2cInstance, address, data, length, false);

    // Release the bus for other tasks
    xSemaphoreGive(mutex);

    return result;
}

/**
 * @brief Reads data from an I2C device
 *
 * The I2C bus is locked during the transaction to prevent another task
 * from accessing it simultaneously.
 *
 * @param address 7-bit I2C address of the target device
 * @param data Buffer where the received data will be stored
 * @param length Number of bytes to read
 *
 * @return Number of bytes read, or a negative value on error
 */
int I2c::read(const uint8_t address, uint8_t *data, const size_t length)
{
    // Wait until the I2C bus is available
    xSemaphoreTake(mutex, portMAX_DELAY);

    const int result = i2c_read_blocking(i2cInstance, address, data, length, false);

    // Release the bus for other tasks
    xSemaphoreGive(mutex);

    return result;
}

/**
 * @brief Writes data to an I2C device and then reads a response
 *
 * Both operations are performed while holding the same mutex, ensuring
 * that no other task can access the bus between the write and read.
 *
 * @param address 7-bit I2C address of the target device
 * @param writeData Pointer to the data to send
 * @param writeLength Number of bytes to send
 * @param readData Buffer where the received data will be stored
 * @param readLength Number of bytes to read
 *
 * @return Number of bytes read if successful, or a negative value on error
 */
int I2c::writeRead(const uint8_t address,
                   const uint8_t *writeData,
                   const size_t writeLength,
                   uint8_t *readData,
                   const size_t readLength)
{
    // Keep the bus locked for the complete write-read transaction
    xSemaphoreTake(mutex, portMAX_DELAY);

    int result = i2c_write_blocking(i2cInstance, address, writeData, writeLength, false);

    // Only attempt the read if the write succeeded
    if (result >= 0) {
        result = i2c_read_blocking(i2cInstance, address, readData, readLength, false);
    }

    // Release the bus for other tasks
    xSemaphoreGive(mutex);

    return result;
}