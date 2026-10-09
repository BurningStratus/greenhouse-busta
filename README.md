# Greenhouse Burta Project

## Description

This project was made as an assignment at Metropolia UAS, Finland. It's a CO2 level controller for a greenhouse. It's
programmed using a Raspberry Pico, a GMP252, a HMP60 and a SDP510 sensor, an OLED screen, a fan and a valve controller.

The goal is to keep the CO2 levels at a targeted level, chosen by the user. When the CO2 levels go over 2000ppm, it will
be brought down to the targeted level through ventilation. When the level is lower than the targeted level, it will be
brought up to the targeted level through opening the valve for 1 second every 30 seconds until the level is reached.

You can find more information inside the [CO2 controller specification](doc/Greenhouse%20CO2_controller_specification.pdf).

## Project Structure

```text
greenhouse-busta/
├── doc/                  # Documentation, datasheets and diagrams
│   ├── diags/            # Architecture and system diagrams
│   └── images/           # Diagrams converted to images
├── pico-sdk/             # Raspberry Pi Pico SDK
├── rp2040-freertos/      # FreeRTOS kernel and dependencies
├── src/                  # Application source code
│   ├── actuators/        # Fan and CO2 valve interface
│   ├── buttons/          # Physical button handling
│   ├── control/          # Fan and CO2 valve control task
│   ├── debug/            # Debugging utilities
│   ├── drivers/          # Hardware communication (UART, I2C, Modbus)
│   ├── sensors/          # Sensor interfaces and measurements
│   ├── shared/           # Shared data structures and configuration
│   ├── ui/               # Display views and user interface task
│   ├── CMakeLists.txt    # Application build configuration
│   ├── FreeRTOSConfig.h  # FreeRTOS configuration
│   └── main.cpp          # Application entry point and initialization
├── CMakeLists.txt        # Main CMake build configuration
├── openocd.cfg           # OpenOCD debugging configuration
└── README.md             # Project documentation
```

## How to flash the program

TBD

Compilation and flashing the Raspberry Pico.

## User manual

TBD

Explain how to use buttons, different view.

## Diagrams

![System diagram](doc/images/system_diagram.png)

## Roles

- Pavel Shishkin: Storing data (EEPROM) and ROM interface, Screen interface
- Pere: Modbus communication, Fan and valve interface, Fan and valve control task
- Fabien Léger: SDP610, GMP and HMP sensors, Buttons, Screen task, Debug task

Dropped down parts:
- EEPROM loading at program start (config gets stored when it changes but never loaded)
- WiFi (ThingSpeak)