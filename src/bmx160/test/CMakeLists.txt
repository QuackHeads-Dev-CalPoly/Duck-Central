add_executable(data_test
    simple_test.cpp
)

# create UF2 files
pico_add_extra_outputs(data_test)
# Enable stdout to USB serial
pico_enable_stdio_usb(data_test 1)
# Disable stdout to UART0
pico_enable_stdio_uart(data_test 0)

target_link_libraries(data_test 
    bmx160
)