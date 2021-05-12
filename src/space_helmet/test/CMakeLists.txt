add_executable(duck_sd_card_test
    sd_card_test.cpp
)

# create UF2 files
pico_add_extra_outputs(duck_sd_card_test)
# Enable stdout to USB serial
pico_enable_stdio_usb(duck_sd_card_test 1)
# Disable stdout to UART0
pico_enable_stdio_uart(duck_sd_card_test 0)

target_link_libraries(duck_sd_card_test
    PRIVATE 
        space_helmet
        hardware_spi 
        hardware_irq
        hardware_gpio
)