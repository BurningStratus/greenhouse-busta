#include "sensors/ModbusSensor.hpp"

#include <cstdint>
#include <cstring>

ModbusSensor::ModbusSensor(Modbus &modbus) : modbus(modbus) {}

bool ModbusSensor::readFloat(const uint8_t deviceAddress,
                             const uint8_t functionCode,
                             const uint16_t registerAddress,
                             float &value) const {
    uint16_t regs[2];

    // Read modbus registers
    if (!modbus.readRegisters(deviceAddress, functionCode, registerAddress, regs, 2)) {
        return false;
    }

    // Convert memory read to float
    const uint32_t raw = (static_cast<uint32_t>(regs[1]) << 16) | static_cast<uint32_t>(regs[0]);
    static_assert(sizeof(value) == sizeof(raw));
    memcpy(&value, &raw, sizeof(value));

    return true;
}


