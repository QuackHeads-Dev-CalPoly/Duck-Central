# Create a library which includes the source files
# Any number of sources could be listed here.
add_library(board_control batt_pwr_and_temp_sensor.c power_controller.cpp)

target_link_libraries(board_control
    PUBLIC
        hardware_gpio
        hardware_adc
        hardware_uart
)

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories (board_control PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})