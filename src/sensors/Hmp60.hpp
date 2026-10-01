#pragma once

#include <cstdint>

#include "drivers/Modbus.h"

class Hmp60 {
public:
    explicit Hmp60(Modbus &modbus);

    bool readHumidity(float &humidity);
    bool readTemperature(float &temperature);
private:
    Modbus &modbus;

    static constexpr uint8_t address = 241;
};
