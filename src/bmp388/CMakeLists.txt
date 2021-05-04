# Create a library which includes the source files
# Any number of sources could be listed here.
add_library(bmp388 bmp_388.cpp)

target_link_libraries(bmp388
    PUBLIC
        hardware_i2c
        hardware_irq
)

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories (bmp388 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(driver)
add_subdirectory(test)
