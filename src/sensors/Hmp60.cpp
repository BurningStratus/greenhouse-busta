#include "Hmp60.hpp"

Hmp60::Hmp60(Modbus &modbus) : ModbusSensor(modbus) {}

bool Hmp60::readHumidity(float &humidity) const
{
    return readFloat(address,modbusReadMultipleHoldingRegisters,humidityRegister,humidity);
}
