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
        hardware_spi 
        hardware_irq
)
