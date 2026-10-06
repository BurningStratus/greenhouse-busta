/**
 *  @file       main.cpp
 *  @authors    Fabien Léger, Pere Joan Garriga Voltas and Pavel Shishkin
 *  @version    0.1
 *  @date       30.09.2026
 *  @link       https://github.com/BurningStratus/greenhouse-busta
 *
 *  @brief      main for the Greenhouse Busta project where we open a fan/valve depending on sensors from the outside
 *              and inside of a chamber to maintain co2 levels inside it to a chosen level
 */

// General c, c++ and FreeRTOS libraries
#include <cstdio>
#include <iostream> // I/O streams for printing
#include <cstring> // Needed for runtime statistics
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "hardware/timer.h"

// Pico related libraries
#include "pico/stdio.h"
#include "hardware/gpio.h"
#include "pico/platform/panic.h"
#include "hardware/i2c.h"

// Self-made drivers
#include "drivers/PicoOsUart.hpp"
#include "drivers/Modbus.h"
#include "drivers/I2c.hpp"
#include "drivers/Display.hpp"

// Shared data
#include "shared/ControlConfig.h"
#include "shared/ControlStatus.h"
#include "shared/ConfigStorage.hpp"
#include "shared/SensorData.h"

// Fan and Valve control
#include "actuators/Fan.h"
#include "actuators/Valve.h"
#include "control/FanValveTask.h"

// Sensors
#include "sensors/SensorReaderTask.hpp"

// Debug
#include "debug/Debug.hpp"

// Buttons
#include "buttons/buttons.hpp"

// UI
#include "ui/UserInterfaceTask.hpp"

// Read the board if you want to check
#define UART_NR         1
#define UART_TX_PIN     4
#define UART_RX_PIN     5
#define UART_SPEED   9600
#define UART_STOP_NR    2

// I2C1 for pressure sensor and OLED
#define I2C1_SDA_PIN       14
#define I2C1_SCL_PIN       15
#define I2C1_BAUD_RATE 100000

#define DISPLAY_I2C_ADDR 0x3C

extern "C" {
uint32_t read_runtime_ctr(void) {
    return timer_hw->timerawl;
}
}

// stack overflow check
extern "C" {
void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName ) {
    if (pcTaskName != nullptr) panic("Stack overflow: %s",pcTaskName);
    else panic("Stack overflow of unnamed task");
}
}

int main() {
    stdio_init_all();

    // create config
    constexpr TickType_t maxAge = pdMS_TO_TICKS(5000);
    constexpr bool valve_open_level = true;

    // Create i2c
    I2c i2c0_con {i2c0, 16, 17, I2C1_BAUD_RATE}; // EEPROM
    I2c i2c1_con {i2c1, I2C1_SDA_PIN, I2C1_SCL_PIN, I2C1_BAUD_RATE}; // pressure/OLED

    // Create config storage (control and state config)
    auto& config_storage = ConfigStorage::instance();

    // 0x50 == address of ROM on I2C bus
    // &i2c == address of i2c object
    // 1000 == general config will be stored at that address in ROM
    // 2000 == fan params will be stored at that address in ROM.
    config_storage.initialize(&i2c0_con, 0x50, 1000, 2000);

    // Keep the latest control status
    QueueHandle_t statusQueue = xQueueCreate(1, sizeof(ControlStatus));
    if (statusQueue == nullptr) {
        panic("Could not create control status queue");
    }

    // Create debug queue for debugging messages
    QueueHandle_t debugQueue = xQueueCreate(10, sizeof(DebugEvent));
    if (debugQueue == nullptr) {
        panic("Could not create debug data queue\n");
    }

    // Create sensor queue for sharing sensor to control
    QueueHandle_t controlSensorQueue = xQueueCreate(5, sizeof(SensorData));
    if (controlSensorQueue == nullptr) {
        panic("Could not create sensor data queue\n");
    }

    // Create sensor queue for sharing sensor to ui
    QueueHandle_t uiSensorQueue = xQueueCreate(1, sizeof(SensorData));
    if (uiSensorQueue == nullptr) {
        panic("Could not create sensor data queue\n");
    }

    // Create queue to send button commands to OLED
    QueueHandle_t buttonQueue = xQueueCreate(5, sizeof(Button));
    if (buttonQueue == nullptr) {
        panic("Could not create user interface queue\n");
    }

    // Create uart and build modbus on top
    PicoOsUart modbusUart{UART_NR, UART_TX_PIN, UART_RX_PIN, UART_SPEED, UART_STOP_NR};
    Modbus modbus{&modbusUart};

    // Initialization of valve and fan
    Valve valve{27,valve_open_level};
    Fan fan{&modbus};

    // Create display
    Display display{i2c1_con, DISPLAY_I2C_ADDR};

    // Create parameters for tasks
    DebugParams debugParams{.debugQueue = &debugQueue};

    SensorParams sensorParams{.modbus             = &modbus,
                              .i2c                = &i2c1_con,
                              .controlSensorQueue = &controlSensorQueue,
                              .uiSensorQueue      = &uiSensorQueue,
                              .debugQueue         = &debugQueue};

    FanValveTaskParams fanValveParams{.sensorQueue = controlSensorQueue,
                                      .valve       = &valve,
                                      .maxAge      = maxAge,
                                      .fan         = &fan,
                                      .statusQueue = statusQueue};

    UserInterfaceParams userInterfaceParams{.sensorQueue = uiSensorQueue,
                                            .buttonQueue = buttonQueue,
                                            .debugQueue  = debugQueue,
                                            .display     = display};

    // Initialize buttons and IRQs
    initButton();
    linkButtonQueueToGpioCallback(buttonQueue); // required for access in gpio_callback()

    printf("\nBoot\n");

    /* TODO REMOVE */
    config_storage.global_config().co2Target = 1000.0f;

    printf("Config store succeeding: %d\n",
           config_storage.config_store());

    config_storage.global_config().co2Target = 1500.0f;

    printf("Before load: %f\n",
           config_storage.global_config().co2Target);

    printf("Config load succeeding: %d\n",
           config_storage.config_load());

    printf("After load: %f\n",
           config_storage.global_config().co2Target);
    /* TODO END REMOVE */

    // create all tasks and start scheduler
    if (xTaskCreate(debugTask,
                    "debug",
                    512,
                    (void *)&debugParams,
                    tskIDLE_PRIORITY + 1,
                    nullptr) != pdPASS) {
        panic("Could not create debugTask() task\n");
    }
    if (xTaskCreate(sensorReader,
                    "sensorReader",
                    512,
                    (void *)&sensorParams,
                    tskIDLE_PRIORITY + 3,
                    nullptr) != pdPASS) {
        panic("Could not create sensorReader() task\n");
    }
    if (xTaskCreate(FanValveTask,
                    "fanValve",
                    512,
                    &fanValveParams,
                    tskIDLE_PRIORITY + 3,
                    nullptr) != pdPASS) {
        panic("Could not create FanValveTask() task\n");
    }
    if (xTaskCreate(userInterfaceTask,
                    "userInterfaceTask",
                    512,
                    &userInterfaceParams,
                    tskIDLE_PRIORITY + 2,
                    nullptr) != pdPASS) {
        panic("Could not create userInterfaceTask() task\n");
    }

    vTaskStartScheduler();

    while(true){};
}
