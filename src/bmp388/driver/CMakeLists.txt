# Create a library which includes the source files
# Any number of sources could be listed here.
add_library(driver bmp3.c bmp3_selftest.c)

# Make sure the compiler can find include files for our library
# when other libraries or executables link to it
target_include_directories (driver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})