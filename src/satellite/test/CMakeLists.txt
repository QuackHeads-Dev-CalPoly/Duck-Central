add_executable(basic_send
    basic_send.cpp
)

# create UF2 files
pico_add_extra_outputs(basic_send)
# Enable stdout to USB serial
pico_enable_stdio_usb(basic_send 1)
# Disable stdout to UART0
pico_enable_stdio_uart(basic_send 0)

target_link_libraries(basic_send 
    PRIVATE 
        satellite
        hardware_uart
)