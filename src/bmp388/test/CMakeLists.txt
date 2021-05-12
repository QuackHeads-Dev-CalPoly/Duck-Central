add_executable(bmp388_test
    bmp388_test.cpp
)

# create UF2 files
pico_add_extra_outputs(bmp388_test)
# Enable stdout to USB serial
pico_enable_stdio_usb(bmp388_test 1)
# Disable stdout to UART0
pico_enable_stdio_uart(bmp388_test 0)

target_link_libraries(bmp388_test 
    PRIVATE 
        bmp388
        driver
        hardware_i2c
        hardware_irq
)