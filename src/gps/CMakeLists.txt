# Create a library which includes the source files
# Any number of sources could be listed here.
add_library (gps gps.cpp minmea.c)

target_link_libraries(gps
    
        hardware_uart
        hardware_dma
        hardware_irq
)

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories (gps PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(test)