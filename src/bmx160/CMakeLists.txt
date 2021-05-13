# Create a library which includes the source files
# Any number of sources could be listed here.
add_library(bmx160 bmx160.cpp)

target_link_libraries(bmx160
    PUBLIC
        hardware_i2c
)

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories (bmx160 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(test)