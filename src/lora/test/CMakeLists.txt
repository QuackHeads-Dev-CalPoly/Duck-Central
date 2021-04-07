add_executable(repeater_test
    duck_lora_repeater.c
)

target_link_libraries(repeater_test 
    PRIVATE 
        lora 
        hardware_spi 
        hardware_irq
)
