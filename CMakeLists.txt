# Generated Cmake Pico project file
cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH /home/knottber/pico/pico-sdk/)

# Pull in Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(duck C CXX)

# Initialise the Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

set( DUCK_WIFI_SRC 
        ${PROJECT_SOURCE_DIR}/duck.c
        ${PROJECT_SOURCE_DIR}/winc1500/bsp/source/nm_bsp_rp2040.c
        ${PROJECT_SOURCE_DIR}/winc1500/bus_wrapper/source/nm_bus_wrapper_rp2040.c
        ${PROJECT_SOURCE_DIR}/winc1500/socket/source/socket.c
        ${PROJECT_SOURCE_DIR}/winc1500/common/source/nm_common.c
        ${PROJECT_SOURCE_DIR}/winc1500/spi_flash/source/flexible_flash.c
        ${PROJECT_SOURCE_DIR}/winc1500/spi_flash/source/spi_flash.c
        ${PROJECT_SOURCE_DIR}/winc1500/bus_wrapper/source/nm_bus_wrapper_rp2040.c
        ${PROJECT_SOURCE_DIR}/winc1500/bsp/source/nm_bsp_rp2040.c
        ${PROJECT_SOURCE_DIR}/winc1500/driver/source/m2m_hif.c
        ${PROJECT_SOURCE_DIR}/winc1500/driver/source/nmbus.c
        ${PROJECT_SOURCE_DIR}/winc1500/driver/source/m2m_ota.c
        ${PROJECT_SOURCE_DIR}/winc1500/driver/source/nmdrv.c
        ${PROJECT_SOURCE_DIR}/winc1500/driver/source/nmuart.c
        ${PROJECT_SOURCE_DIR}/winc1500/driver/source/m2m_wifi.c
        ${PROJECT_SOURCE_DIR}/winc1500/driver/source/nmspi.c
        ${PROJECT_SOURCE_DIR}/winc1500/driver/source/nmi2c.c
        ${PROJECT_SOURCE_DIR}/winc1500/driver/source/m2m_crypto.c
        ${PROJECT_SOURCE_DIR}/winc1500/driver/source/m2m_ate_mode.c
        ${PROJECT_SOURCE_DIR}/winc1500/driver/source/m2m_periph.c
        ${PROJECT_SOURCE_DIR}/winc1500/driver/source/nmasic.c
        ${PROJECT_SOURCE_DIR}/winc1500/driver/source/m2m_ssl.c
        )

add_executable(duck ${DUCK_WIFI_SRC})

include_directories(duck ${PROJECT_SOURCE_DIR}/winc1500)

pico_set_program_name(duck "duck")
pico_set_program_version(duck "0.1")

pico_enable_stdio_usb(duck 1)

# Add the standard library to the build
target_link_libraries(duck pico_stdlib)

# Add any user reuqested libraries
target_link_libraries(duck
        hardware_i2c
        hardware_spi
        )

pico_add_extra_outputs(duck)
