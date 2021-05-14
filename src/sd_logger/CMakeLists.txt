# Create a library which includes the source files
# Any number of sources could be listed here.
add_library(space_helmet sd_logger.cpp)

target_link_libraries(space_helmet
    PUBLIC
        hardware_spi
        hardware_irq
        hardware_gpio
)

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories (space_helmet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(test)