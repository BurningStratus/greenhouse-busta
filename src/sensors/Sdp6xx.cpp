#include "Sdp6xx.hpp"

/**
 * @brief Creates an SDP6xx pressure sensor using an existing I2C connection
 *
 * @param i2c I2C connection used to communicate with the sensor
 */
Sdp6xx::Sdp6xx(I2c &i2c) : i2c(i2c) {}

/**
 * @brief Reads the differential pressure measured by the sensor
 *
 * Sends the measurement command to the sensor and reads the returned
 * signed 16-bit measurement. The raw value is then converted to Pascals
 * using the sensor scale factor.
 *
 * @param pressure Reference to save the pressure to
 *
 * @return Number of bytes written, or a negative value on error
 */
int Sdp6xx::readPressure(float &pressure) const
{
    uint8_t data[2];
    int result;

    // Send a command and read the pressure data received
    if ((result = i2c.writeRead(address, &measureCommand, 1, data, 2)) < 0) {
        return result;
    }

    // Combine MSB and LSB a uint16_t
    const auto rawPressure = static_cast<int16_t>((static_cast<uint16_t>(data[0]) << 8) | data[1]);

    // Convert the raw sensor value to pascals
    pressure = static_cast<float>(rawPressure) / scaleFactor;

    return result;
}
