#pragma once

#include <cstdint>

#include "drivers/Modbus.h"
#include "sensors/ModbusSensor.hpp"

class Gmp252 : public ModbusSensor {
public:
    explicit Gmp252(Modbus &modbus);

    bool readCo2(float &co2) const;
    bool readTemperature(float &temperature) const;

private:
    static constexpr uint8_t deviceAddress = 240;

    static constexpr uint16_t co2Register = 0x0000;
    static constexpr uint16_t temperatureRegister = 0x0004;
};