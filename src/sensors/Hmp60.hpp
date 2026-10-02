#pragma once

#include <cstdint>

#include "drivers/Modbus.h"
#include "sensors/ModbusSensor.hpp"

class Hmp60 : public ModbusSensor {
public:
    explicit Hmp60(Modbus &modbus);

    bool readHumidity(float &humidity) const;

private:
    static constexpr uint8_t address = 241;
    static constexpr uint16_t humidityRegister = 0x0000;
};