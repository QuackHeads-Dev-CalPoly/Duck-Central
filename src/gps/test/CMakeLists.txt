add_executable(gps_test
    duck_simple_gps_test.cpp
)

target_link_libraries(gps_test
    PRIVATE
        gps
        hardware_uart
        pico_mem_ops
        pico_malloc
        pico_runtime
        hardware_dma
)