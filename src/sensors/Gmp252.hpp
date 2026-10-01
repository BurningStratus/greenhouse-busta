#pragma once

#include <cstdint>

#include "drivers/Modbus.h"

class Gmp252 {
public:
    explicit Gmp252(Modbus &modbus);

    bool readCo2(float &co2);
    bool readTemperature(float &temperature);
private:
    Modbus &modbus;

    static constexpr uint8_t address = 240;
};
