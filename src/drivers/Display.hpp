#pragma once
#include "mono_vlsb.h"
#include "I2c.hpp"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 

class Display : public mono_vlsb {
public:
    explicit Display
    (I2c& i2c_instance, uint8_t i2c_addr, uint16_t width = SCREEN_WIDTH, uint16_t height = SCREEN_HEIGHT)
        : mono_vlsb(width, height, width, 1), bus (i2c_instance), bus_addr (i2c_addr)
    {
        // Constructor allocates buffer that is one bigger than what is needed.
        // The extra byte is needed for the command byte when updating the display.
        // Height must be multiple of 8.

        // set control byte at the beginning of frame buffer
        buffer.get()[0] = 0x40;
        init();
    }

    void show ();
    void show (framebuf&);
private:
    void init ();
    void cmd (uint8_t value);
    
    I2c& bus;
    uint8_t bus_addr;
};

// template for a 'screen'
struct sensor_view : public framebuf {
    sensor_view (uint16_t width, uint16_t height) 
        : framebuf (SCREEN_WIDTH, SCREEN_HEIGHT)
    {}

    void generate (float co2, float hum, float tmp, float pressure)
    {
        buffer.
    }
};
// template for a 'screen'
struct params_view : public framebuf {
    
};
