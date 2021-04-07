# Create a library which includes the source files
# Any number of sources could be listed here.
add_library(lora lora.c)

target_link_libraries(lora
    PUBLIC
        hardware_spi
        hardware_irq
)

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories (lora PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(test)