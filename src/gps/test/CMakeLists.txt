add_executable(test_gps
    test_gps.cpp
)

# create UF2 files
pico_add_extra_outputs(test_gps)
# Enable stdout to USB serial
pico_enable_stdio_usb(test_gps 1)
# Disable stdout to UART0
pico_enable_stdio_uart(test_gps 0)

target_link_libraries(test_gps 
    PRIVATE 
        gps 
)