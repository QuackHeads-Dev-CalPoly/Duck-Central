# Create a library which includes the source files
# Any number of sources could be listed here.
add_library(satellite IridiumSBD.cpp)

target_link_libraries(satellite
    PUBLIC
        hardware_uart
)

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories (satellite PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(test)