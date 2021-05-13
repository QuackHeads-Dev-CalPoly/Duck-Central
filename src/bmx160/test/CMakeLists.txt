add_executable(pop_test
    pop_test.cpp
)

# create UF2 files
pico_add_extra_outputs(pop_test)
# Enable stdout to USB serial
pico_enable_stdio_usb(pop_test 1)
# Disable stdout to UART0
pico_enable_stdio_uart(pop_test 0)

target_link_libraries(pop_test 
    bmx160
)