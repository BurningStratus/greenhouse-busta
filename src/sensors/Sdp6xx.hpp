#pragma once

#include <cstdint>

#include "drivers/I2c.hpp"

class Sdp6xx {
public:
    // Constructor
    explicit Sdp6xx(I2c &i2c);

    // Copy
    Sdp6xx(const Sdp6xx &) = delete;
    Sdp6xx &operator=(const Sdp6xx &) = delete;

    // Methods
    int readPressure(float &pressure) const;
private:
    // Constants
    static constexpr uint8_t address = 0x40;
    static constexpr uint8_t measureCommand = 0xF1;
    static constexpr float scaleFactor = 240.0f;

    // Attributes
    I2c &i2c;
};