add_executable(repeater_test
    duck_lora_repeater.cpp
)

# create UF2 files
pico_add_extra_outputs(repeater_test)
# Enable stdout to USB serial
pico_enable_stdio_usb(repeater_test 1)
# Disable stdout to UART0
pico_enable_stdio_uart(repeater_test 0)

target_link_libraries(repeater_test 
    PRIVATE 
        lora 
        board_control
        hardware_spi 
        hardware_irq
)

#------------------------------------------------------------------------------

add_executable(transmitter_test
    duck_lora_transmitter.cpp
)

# create UF2 files
pico_add_extra_outputs(transmitter_test)
# Enable stdout to USB serial
pico_enable_stdio_usb(transmitter_test 1)
# Disable stdout to UART0
pico_enable_stdio_uart(transmitter_test 0)

target_link_libraries(transmitter_test 
    PRIVATE 
        lora 
        board_control
        hardware_spi 
        hardware_irq
)

#------------------------------------------------------------------------------

add_executable(ground_station
    duck_lora_receiver.cpp
)

# create UF2 files
pico_add_extra_outputs(ground_station)
# Enable stdout to USB serial
pico_enable_stdio_usb(ground_station 1)
# Disable stdout to UART0
pico_enable_stdio_uart(ground_station 0)

target_link_libraries(ground_station 
    PRIVATE 
        lora 
        board_control
        hardware_spi 
        hardware_irq
)

#------------------------------------------------------------------------------