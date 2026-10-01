add_executable(${ProjectName}
        main.cpp
        actuators/Fan.cpp
        actuators/Fan.h
        actuators/Valve.cpp
        actuators/Valve.h
        control/FanValveTask.cpp
        control/FanValveTask.h
        shared/SensorData.h
        shared/ControlStatus.h
        shared/ControlConfig.h
        drivers/PicoOsUart.cpp
        drivers/PicoOsUart.hpp
        drivers/Modbus.cpp
        drivers/Modbus.h
        drivers/I2c.cpp
        drivers/I2c.hpp
        sensors/Sdp6xx.cpp
        sensors/Sdp6xx.hpp
        sensors/Gmp252.cpp
        sensors/Gmp252.hpp
        sensors/Hmp60.cpp
        sensors/Hmp60.hpp
)

target_compile_definitions(${ProjectName} PRIVATE
)

target_link_libraries(${ProjectName} 
        pico_stdlib
        hardware_i2c
        FreeRTOS-Kernel-Heap4
)

target_include_directories(${ProjectName} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(${ProjectName})

# Disable usb output, enable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)
