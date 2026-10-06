#include "ControlConfig.h"
#include "drivers/ROM.hpp"
#include "pico/platform/panic.h"

#pragma once

class ConfigStorage {
public:
    void initialize
    (I2c* ptr, int i2c_addr, int cfg_rom_addr, int fan_rom_addr)
    {
        if (init_done)
            return; // means this object is initialized

        static ROM rom_ref (*ptr, i2c_addr);
        memory = &rom_ref;

        global_config_address = static_cast <uint16_t> (cfg_rom_addr);
        fan_config_address =    static_cast <uint16_t> (fan_rom_addr);

        init_done = true;
    }

    ConfigStorage& operator=(const ConfigStorage& other) = delete;

    static ConfigStorage& instance ()
    {
        static ConfigStorage storage_instance;
        return storage_instance;
    }

    bool config_load ();
    bool config_store () const;

    ControlConfig& global_config ()
    {
        return global_cfg;
    }
    ControlState& global_state ()
    {
        return fan_cfg;
    }

private:
    bool init_done; // you cannot use object before you initialize it.

    ROM * memory;
    ControlConfig global_cfg;
    ControlState fan_cfg;
    
    uint16_t global_config_address;
    uint16_t fan_config_address;

    ConfigStorage()
    : init_done(false),
      memory(nullptr),
      global_config_address(0),
      fan_config_address(0)
    {}
};
