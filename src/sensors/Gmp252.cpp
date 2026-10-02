#include "Gmp252.hpp"

Gmp252::Gmp252(Modbus &modbus) : ModbusSensor(modbus) {}

bool Gmp252::readCo2(float &co2) const
{
    return readFloat(deviceAddress,modbusReadMultipleHoldingRegisters,co2Register,co2);
}

bool Gmp252::readTemperature(float &temperature) const
{
    return readFloat(deviceAddress,modbusReadMultipleHoldingRegisters,temperatureRegister,temperature);
}
