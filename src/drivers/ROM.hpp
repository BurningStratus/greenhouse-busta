#pragma once

// interface EEPROM, to store data.
#include "I2c.hpp"
#include <string.h>

class ROM {
public:
    explicit ROM (I2c& i2c_ref, int i2c_addr)
    : i2c_bus (i2c_ref), i2c_addr (static_cast <uint8_t> (i2c_addr))
    {}

    // int writepg (uint16_t rom_addr, const uint8_t *payload, int payload_size)
    size_t store (uint16_t rom_addr, const uint8_t* payload, size_t payload_size);

    // uint8_t *seqread (uint16_t rom_addr, int wcnt)
    size_t load (uint16_t rom_addr, uint8_t *dst, size_t length);

private:
    I2c& i2c_bus;
    uint8_t i2c_addr;
};

