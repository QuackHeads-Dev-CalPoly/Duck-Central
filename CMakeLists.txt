# Generated Cmake Pico project file
cmake_minimum_required(VERSION 3.12)

set(PROJECT_VER_MAJOR 1)
set(PROJECT_VER_MINOR 0)
set(PROJECT_VER_PATCH 0)
set(PROJECT_VER_STR "${PROJECT_VER_MAJOR}.${PROJECT_VER_MINOR}.${PROJECT_VER_PATCH}")
message("Building Version: " ${PROJECT_VER_STR})

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})

# Pull in Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(duck C CXX)

# Initialize the Pico SDK
pico_sdk_init()

# Do this before adding subdirs so we don't have to include it in every build file
link_libraries(pico_stdlib)

# Include the generic includes for everybody
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Finally, add sub directories after configuration is complete
add_subdirectory(src)
add_subdirectory(tst) 