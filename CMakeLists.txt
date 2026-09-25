cmake_minimum_required(VERSION 3.13)

# Set board type because we are building for PicoW
set(PICO_BOARD pico_w)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

# Name project
set(ProjectName greenhouse-busta)

# Set any variables required for importing libraries
set(PICO_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk)
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/FreeRTOS-KernelV10.6.2)
set(PICOTOOL_FETCH_FROM_GIT 1)

# Import those libraries
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

# Define project
project(${ProjectName} C CXX ASM)

# Initialize the Raspberry Pi Pico SDK
pico_sdk_init()

#add_compile_options(-O0)
# Add subdirectories
add_subdirectory(src)

# if you have anything in "lib" folder then uncomment below - remember to add a CMakeLists.txt
# file to the "lib" directory
#add_subdirectory(lib)

