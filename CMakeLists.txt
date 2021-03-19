# Generated Cmake Pico project file
cmake_minimum_required(VERSION 3.12)

set(PROJECT_VER_MAJOR 1)
set(PROJECT_VER_MINOR 0)
set(PROJECT_VER_PATCH 0)
set(PROJECT_VER_STR "${PROJECT_VER_MAJOR}.${PROJECT_VER_MINOR}.${PROJECT_VER_PATCH}")
message("Building Version: " ${PROJECT_VER_STR})

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})

# Pull in Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(duck C CXX)

# Initialise the Pico SDK
pico_sdk_init()

set( DUCK_QUACKER_BOARD 
        ${PROJECT_SOURCE_DIR}/quacker_board/batt_pwr_and_temp_sensor.c
        )

set( DUCK_GPS_COMMON 
        ${PROJECT_SOURCE_DIR}/gps/gps.c
        ${PROJECT_SOURCE_DIR}/gps/minmea.c
        )

set( DUCK_LORA_COMMON 
        ${PROJECT_SOURCE_DIR}/lora/lora.c
        )

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

add_executable(duck_chip_info_and_scan ${PROJECT_SOURCE_DIR}/test/duck_chip_info_and_scan_exp.cpp ${DUCK_WIFI_COMMON})
add_executable(duck_simple_web_server  ${PROJECT_SOURCE_DIR}/test/duck_simple_web_server_exp.cpp ${DUCK_WIFI_COMMON})
add_executable(duck_ap_simple_web_server  ${PROJECT_SOURCE_DIR}/test/duck_ap_simple_web_server_exp.cpp ${DUCK_WIFI_COMMON})
add_executable(duck_ap_gps_lora_demo_1 ${PROJECT_SOURCE_DIR}/test/duck_ap_gps_lora_demo_1.cpp 
                                                ${DUCK_WIFI_COMMON} ${DUCK_LORA_COMMON} ${DUCK_GPS_COMMON} ${DUCK_QUACKER_BOARD})
add_executable(duck_simple_gps_test ${PROJECT_SOURCE_DIR}/test/duck_simple_gps_test.cpp ${DUCK_GPS_COMMON})
add_executable(duck_simple_blink_log ${PROJECT_SOURCE_DIR}/test/duck_simple_blink_out.c)
add_executable(duck_adc_temp_and_bat_sense ${PROJECT_SOURCE_DIR}/test/duck_adc_temp_and_bat_sense.cpp ${DUCK_QUACKER_BOARD}) 
add_executable(duck_lora_repeater ${PROJECT_SOURCE_DIR}/test/duck_lora_repeater.c ${DUCK_LORA_COMMON})
add_executable(duck_lora_message_logger ${PROJECT_SOURCE_DIR}/test/duck_lora_message_logger.cpp ${DUCK_LORA_COMMON})
add_executable(duck_gps_dma ${PROJECT_SOURCE_DIR}/test/duck_gps_dma.cpp ${DUCK_GPS_COMMON})

include_directories(duck_chip_info_and_scan ${PROJECT_SOURCE_DIR}/winc1500)
include_directories(duck_simple_web_server ${PROJECT_SOURCE_DIR}/winc1500)
include_directories(duck_ap_simple_web_server ${PROJECT_SOURCE_DIR}/winc1500)
include_directories(duck_ap_gps_lora_demo_1 ${PROJECT_SOURCE_DIR}/lora ${PROJECT_SOURCE_DIR}/winc1500 ${PROJECT_SOURCE_DIR}/quacker_board)
include_directories(duck_simple_gps_test ${PROJECT_SOURCE_DIR}/gps)
include_directories(duck_lora_repeater ${PROJECT_SOURCE_DIR}/lora)
include_directories(duck_lora_message_logger ${PROJECT_SOURCE_DIR}/lora)
include_directories(duck_gps_dma ${PROJECT_SOURCE_DIR}/gps)
include_directories(duck_adc_temp_and_bat_sense ${PROJECT_SOURCE_DIR}/quacker_board)

pico_set_program_name(duck_chip_info_and_scan "duck_info_scan")
pico_set_program_version(duck_chip_info_and_scan ${PROJECT_VER_STR})

pico_set_program_name(duck_simple_web_server "duck_webserver")
pico_set_program_version(duck_simple_web_server ${PROJECT_VER_STR})

pico_set_program_name(duck_ap_simple_web_server "duck_webserver")
pico_set_program_version(duck_ap_simple_web_server ${PROJECT_VER_STR})

pico_set_program_name(duck_ap_gps_lora_demo_1 "duck_ap_gps_lora_demo_1")
pico_set_program_version(duck_ap_gps_lora_demo_1 ${PROJECT_VER_STR})

pico_set_program_name(duck_simple_gps_test "duck_simple_gps_test")
pico_set_program_version(duck_simple_gps_test ${PROJECT_VER_STR})

pico_set_program_name(duck_simple_blink_log "duck_simple_blink_log")
pico_set_program_version(duck_simple_blink_log ${PROJECT_VER_STR})

pico_set_program_name(duck_adc_temp_and_bat_sense "duck_adc_temp_and_bat_sense")
pico_set_program_version(duck_adc_temp_and_bat_sense ${PROJECT_VER_STR})

pico_set_program_name(duck_lora_repeater "duck_lora_repeater")
pico_set_program_version(duck_lora_repeater ${PROJECT_VER_STR})

pico_set_program_name(duck_lora_repeater "duck_lora_message_logger")
pico_set_program_version(duck_lora_repeater ${PROJECT_VER_STR})

pico_set_program_name(duck_gps_dma "duck_gps_dma")
pico_set_program_version(duck_gps_dma ${PROJECT_VER_STR})

# Enable stdout to USB serial
pico_enable_stdio_usb(duck_chip_info_and_scan 1)
pico_enable_stdio_usb(duck_simple_web_server 1)
pico_enable_stdio_usb(duck_ap_simple_web_server 1)
pico_enable_stdio_usb(duck_ap_gps_lora_demo_1 1)
pico_enable_stdio_usb(duck_simple_gps_test 1)
pico_enable_stdio_usb(duck_simple_blink_log 1)
pico_enable_stdio_usb(duck_adc_temp_and_bat_sense 1)
pico_enable_stdio_usb(duck_lora_repeater 1)
pico_enable_stdio_usb(duck_lora_message_logger 1)
pico_enable_stdio_usb(duck_gps_dma 1)

# Disable stdout to UART0
pico_enable_stdio_uart(duck_chip_info_and_scan 0)
pico_enable_stdio_uart(duck_simple_web_server 0)
pico_enable_stdio_uart(duck_ap_simple_web_server 0)
pico_enable_stdio_uart(duck_ap_gps_lora_demo_1 0)
pico_enable_stdio_uart(duck_simple_gps_test 0)
pico_enable_stdio_uart(duck_simple_blink_log 0)
pico_enable_stdio_uart(duck_adc_temp_and_bat_sense 0)
pico_enable_stdio_uart(duck_lora_repeater 0)
pico_enable_stdio_uart(duck_lora_message_logger 0)
pico_enable_stdio_uart(duck_gps_dma 0)

# Add the standard library to the build
target_link_libraries(duck_chip_info_and_scan pico_stdlib)
target_link_libraries(duck_simple_web_server pico_stdlib)
target_link_libraries(duck_ap_simple_web_server pico_stdlib)
target_link_libraries(duck_ap_gps_lora_demo_1 pico_stdlib)
target_link_libraries(duck_simple_gps_test pico_stdlib)
target_link_libraries(duck_simple_blink_log pico_stdlib)
target_link_libraries(duck_adc_temp_and_bat_sense pico_stdlib)
target_link_libraries(duck_lora_repeater pico_stdlib)
target_link_libraries(duck_lora_message_logger pico_stdlib)
target_link_libraries(duck_gps_dma pico_stdlib)

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

target_link_libraries(duck_ap_gps_lora_demo_1
        hardware_uart
        hardware_i2c
        hardware_spi
        hardware_adc
        hardware_dma
        pico_mem_ops
        pico_malloc
        pico_runtime
        )

target_link_libraries(duck_simple_gps_test
        hardware_uart
        pico_mem_ops
        pico_malloc
        pico_runtime
        hardware_dma
        )

target_link_libraries(duck_adc_temp_and_bat_sense
        hardware_gpio
        hardware_adc
        hardware_uart
        )

target_link_libraries(duck_lora_repeater
        hardware_gpio
        hardware_spi
        )

target_link_libraries(duck_lora_message_logger
        hardware_gpio
        hardware_spi
)

target_link_libraries(duck_gps_dma
        hardware_uart
        hardware_irq
        hardware_dma
        )       

pico_add_extra_outputs(duck_chip_info_and_scan)
pico_add_extra_outputs(duck_simple_web_server)
pico_add_extra_outputs(duck_ap_simple_web_server)
pico_add_extra_outputs(duck_ap_gps_lora_demo_1)
pico_add_extra_outputs(duck_simple_gps_test)
pico_add_extra_outputs(duck_simple_blink_log)
pico_add_extra_outputs(duck_adc_temp_and_bat_sense)
pico_add_extra_outputs(duck_lora_repeater)
pico_add_extra_outputs(duck_lora_message_logger)
pico_add_extra_outputs(duck_gps_dma)