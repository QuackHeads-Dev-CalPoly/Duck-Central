# Generated Cmake Pico project file
cmake_minimum_required(VERSION 3.12)

set(PROJECT_VER_MAJOR 1)
set(PROJECT_VER_MINOR 0)
set(PROJECT_VER_PATCH 0)
set(PROJECT_VER_STR "${PROJECT_VER_MAJOR}.${PROJECT_VER_MINOR}.${PROJECT_VER_PATCH}")
message("Buidling Version: " ${PROJECT_VER_STR})

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

set( DUCK_WIFI_COMMON 
        ${PROJECT_SOURCE_DIR}/winc1500/bsp/source/nm_bsp_rp2040.c
        ${PROJECT_SOURCE_DIR}/winc1500/bus_wrapper/source/nm_bus_wrapper_rp2040.c
        ${PROJECT_SOURCE_DIR}/winc1500/socket/source/socket.c
        ${PROJECT_SOURCE_DIR}/winc1500/common/source/nm_common.c
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
        ${PROJECT_SOURCE_DIR}/winc1500/utility/WiFiSocket.cpp
        ${PROJECT_SOURCE_DIR}/winc1500/WiFi.cpp
        ${PROJECT_SOURCE_DIR}/winc1500/WiFiClient.cpp
        ${PROJECT_SOURCE_DIR}/winc1500/WiFiMDNSResponder.cpp
        ${PROJECT_SOURCE_DIR}/winc1500/WiFiUdp.cpp
        ${PROJECT_SOURCE_DIR}/winc1500/WiFiServer.cpp
        ${PROJECT_SOURCE_DIR}/winc1500/WiFiSSLClient.cpp
        ${PROJECT_SOURCE_DIR}/winc1500/IPAddress.cpp
        ${PROJECT_SOURCE_DIR}/winc1500/Ard_to_Pico.c
        )

add_compile_definitions(VERSION_STR=${PROJECT_VER_STR})

add_executable(duck_chip_info_and_scan ${PROJECT_SOURCE_DIR}/duck_chip_info_and_scan_exp.cpp ${DUCK_WIFI_COMMON})
add_executable(duck_simple_web_server  ${PROJECT_SOURCE_DIR}/duck_simple_web_server_exp.cpp ${DUCK_WIFI_COMMON})
add_executable(duck_ap_simple_web_server  ${PROJECT_SOURCE_DIR}/duck_ap_simple_web_server_exp.cpp ${DUCK_WIFI_COMMON})

include_directories(duck_chip_info_and_scan ${PROJECT_SOURCE_DIR}/winc1500)
include_directories(duck_simple_web_server ${PROJECT_SOURCE_DIR}/winc1500)
include_directories(duck_ap_simple_web_server ${PROJECT_SOURCE_DIR}/winc1500)

pico_set_program_name(duck_chip_info_and_scan "duck_info_scan")
pico_set_program_version(duck_chip_info_and_scan ${PROJECT_VER_STR})

pico_set_program_name(duck_simple_web_server "duck_webserver")
pico_set_program_version(duck_simple_web_server ${PROJECT_VER_STR})

pico_set_program_name(duck_ap_simple_web_server "duck_webserver")
pico_set_program_version(duck_ap_simple_web_server ${PROJECT_VER_STR})

pico_enable_stdio_usb(duck_chip_info_and_scan 1)
pico_enable_stdio_usb(duck_simple_web_server 1)
pico_enable_stdio_usb(duck_ap_simple_web_server 1)

# Add the standard library to the build
target_link_libraries(duck_chip_info_and_scan pico_stdlib)
target_link_libraries(duck_simple_web_server pico_stdlib)
target_link_libraries(duck_ap_simple_web_server pico_stdlib)

# Add any user reuqested libraries
target_link_libraries(duck_chip_info_and_scan
        hardware_uart
        hardware_i2c
        hardware_spi
        pico_mem_ops
        pico_malloc
        pico_runtime
        )

target_link_libraries(duck_simple_web_server
        hardware_uart
        hardware_i2c
        hardware_spi
        pico_mem_ops
        pico_malloc
        pico_runtime
        )

target_link_libraries(duck_ap_simple_web_server
        hardware_uart
        hardware_i2c
        hardware_spi
        pico_mem_ops
        pico_malloc
        pico_runtime
        )

pico_add_extra_outputs(duck_chip_info_and_scan)
pico_add_extra_outputs(duck_simple_web_server)
pico_add_extra_outputs(duck_ap_simple_web_server)