#pragma once

#include "drivers/Modbus.h"

class ModbusSensor {
public:
    explicit ModbusSensor(Modbus &modbus);

protected:
    bool readFloat(uint8_t deviceAddress,
                   uint8_t functionCode,
                   uint16_t registerAddress,
                   float &value) const;

    Modbus &modbus;
};