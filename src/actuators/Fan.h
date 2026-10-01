#pragma once

#include "drivers/Modbus.h"

class Fan
{
    public:
        bool setSpeed(unsigned int speed);
        Fan(Modbus* modbus);
        bool readPulses(uint16_t& pulses);
    private:
        Modbus* modbus;
};