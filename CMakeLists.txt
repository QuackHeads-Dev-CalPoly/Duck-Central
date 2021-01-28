# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH /Users/krdixson/Desktop/pico/pico-sdk)

# Pull in Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(duck C CXX)

# Initialise the Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(duck duck.c gps.c gps.h minmea.c minmea.h)

pico_set_program_name(duck "duck")
pico_set_program_version(duck "0.1")

# enable usb output, disable uart output
pico_enable_stdio_usb(duck 1)
pico_enable_stdio_uart(duck 0)

# Add the standard library to the build
target_link_libraries(duck pico_stdlib)

# Add any user reuqested libraries
target_link_libraries(duck
        hardware_i2c
        hardware_spi
        )

pico_add_extra_outputs(duck)

