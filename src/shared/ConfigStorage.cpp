#include "ConfigStorage.hpp"

bool ConfigStorage::config_store () const {
    if (!init_done) // you cannot use object before you initialize it.
    {
        panic ("ROM is not initialized, cannot store config.");
        return false;
    }

    size_t bytes = memory->store
    (global_config_address, reinterpret_cast<const uint8_t *> (&global_cfg), sizeof (ControlConfig));

    bytes += memory->store
    (fan_config_address, reinterpret_cast<const uint8_t *> (&fan_cfg), sizeof (ControlState));

    if (bytes != ( sizeof (ControlConfig) + sizeof (ControlState) ))
        return false;

    return true;
}

bool ConfigStorage::config_load ()
{
    if (!init_done) // you cannot use object before you initialize it.
    {
        panic ("ROM is not initialized, cannot load config.");
        return false;
    }

    size_t config_size = sizeof (ControlConfig);
    size_t fan_config_size = sizeof (ControlState);

    size_t bytes = memory->load
    (global_config_address, reinterpret_cast<uint8_t *>(&global_cfg), config_size);

    bytes += memory->load
    (fan_config_address, reinterpret_cast<uint8_t *>(&fan_cfg), fan_config_size);

    if (bytes != ( config_size + fan_config_size ))
        return false;

    return true; 
}
