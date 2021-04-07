#--------------------------- Range Test -----------------------------#
add_executable(range
    test_range.c
)
target_link_libraries(range 
    PRIVATE
        lora
        gps
        board_control
        hardware_spi 
        hardware_uart 
        hardware_gpio
)
# create UF2 files
pico_add_extra_outputs(range)
# Enable stdout to USB serial
pico_enable_stdio_usb(range 1)
# Disable stdout to UART0
pico_enable_stdio_uart(range 0)



#--------------------------- Blink Test -----------------------------#
add_executable(blink
    test_blink.c
)
# create UF2 files
pico_add_extra_outputs(blink)
# Enable stdout to USB serial
pico_enable_stdio_usb(blink 1)
# Disable stdout to UART0
pico_enable_stdio_uart(blink 0)


#--------------------------- ADC Test -----------------------------#
add_executable(adc_batt
    test_adc_temp_and_bat_sense.cpp
)
target_link_libraries(adc_batt 
    PRIVATE 
        board_control
        hardware_gpio
        hardware_adc
        hardware_uart
)
# create UF2 files
pico_add_extra_outputs(adc_batt)
# Enable stdout to USB serial
pico_enable_stdio_usb(adc_batt 1)
# Disable stdout to UART0
pico_enable_stdio_uart(adc_batt 0)


# Note, this requires implementing CMake on our winc1500 library
# #--------------------------- Demo -----------------------------#
# add_executable(demo
#     duck_ap_gps_lora_demo.cpp
# )
# target_link_libraries(demo 
#     PRIVATE 
#         hardware_uart
#         hardware_i2c
#         hardware_spi
#         hardware_adc
#         hardware_dma
#         pico_mem_ops
#         pico_malloc
#         pico_runtime
# )

# # create UF2 files
# pico_add_extra_outputs(blink)
