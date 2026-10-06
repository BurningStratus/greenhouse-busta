#pragma once

#include <cstdio>

#include "display/mono_vlsb.h"
#include "I2c.hpp"
#include "shared/SensorData.h"
#include "shared/ConfigStorage.hpp"

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

class Display : public mono_vlsb {
public:
    explicit Display(I2c& i2c_instance,
                     uint8_t i2c_addr,
                     uint16_t width = SCREEN_WIDTH,
                     uint16_t height = SCREEN_HEIGHT)
        : mono_vlsb(width, height, width, 1),
          bus(i2c_instance),
          bus_addr(i2c_addr)
    {
        buffer.get()[0] = 0x40;
        init();
    }

    void show();

private:
    void init();
    void cmd(uint8_t value);

    I2c& bus;
    uint8_t bus_addr;
};

// Rendering of the display
struct SensorView : public mono_vlsb {
public:
    SensorView() : mono_vlsb(SCREEN_WIDTH, SCREEN_HEIGHT) {}

    void generate(const SensorData& data)
    {
        fill(0);

        char line[32];

        snprintf(line, sizeof(line), "CO2: %.0f ppm", data.co2.value);
        text(line, 0, 0);

        snprintf(line, sizeof(line), "RH: %.1f %%", data.humidity.value);
        text(line, 0, 16);

        snprintf(line, sizeof(line), "Temp: %.1f C", data.temperature.value);
        text(line, 0, 32);

        snprintf(line, sizeof(line), "Press: %.2f Pa", data.pressure.value);
        text(line, 0, 48);
    }
};

struct ParamsView : public mono_vlsb {
public:
    explicit ParamsView() : mono_vlsb(SCREEN_WIDTH, SCREEN_HEIGHT) {}

    void generate(const ControlConfig& config)
    {
        fill(0);

        char line[32];

        text("Configuration", 0, 0);

        snprintf(line, sizeof(line), "CO2 target: %.0f ppm", config.co2Target);
        text(line, 0, 16);
    }
};