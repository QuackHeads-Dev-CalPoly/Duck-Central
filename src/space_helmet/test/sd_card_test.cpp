extern "C"
{
    #include "pico/stdio.h"
    #include "pico/stdlib.h"
    #include <stdio.h>
    #include <string.h>
}

#include "sd_logger.h"

int main()
{
    stdio_init_all();
    sleep_ms(2 * 1000);
    printf("Turned on stdio to stdout\n");
    printf("Sleeping for 10 seconds");
    sleep_ms(10 * 1000);

    printf("Size of double: %d\n", sizeof(double));
    printf("Size of float: %d\n", sizeof(float));
    printf("Size of int16_t: %d\n", sizeof(int16_t));

    sd_logger my_sd;
    my_sd.start_logger(0); // Start at block address zero

    // printf("Initing SD card\n");
    // my_sd.init_sd_card();

    // // Do initial read to have starting bytes of address
    // uint8_t read_buffer[512];
    // my_sd.read_block(0, read_buffer);
    // printf("Read Result 1 \n");
    // for(int i = 0; i < 25; i++)
    //     printf("0x%02x\n", read_buffer[i]);
    
    // // Write bytes to block
    // uint8_t write_buffer[512];
    // uint8_t test_string[28];
    // uint8_t val = 100;
    // for( int i = 0; i < 28; i++ )
    //     test_string[i] = val++;

    // test_string[26] = 'B';
    // test_string[27] = '0';   
    // memset(write_buffer, 0x00, 512); // Clear out write buffer
    // memcpy(write_buffer, test_string, 28);
    // my_sd.write_block(0, write_buffer);
    // printf("Write Result\n");

    // // Read again to check the bytes got updated fully
    // my_sd.read_block(0, read_buffer);
    // printf("Read Result 2 \n");
    // for(int i = 0; i < 28; i++)
    //     printf("0x%02x\n", read_buffer[i]);

    // // Read again to check the bytes got updated fully
    // my_sd.read_block(1, read_buffer);
    // printf("Read Result 2 \n");
    // for(int i = 0; i < 28; i++)
    //     printf("0x%02x\n", read_buffer[i]);

    // test_string[26] = 'B';
    // test_string[27] = '1';
    // memcpy(write_buffer, test_string, 28);
    // my_sd.write_block(1, write_buffer);
    // printf("Write Result\n");

    // // Read again to check the bytes got updated fully
    // my_sd.read_block(1, read_buffer);
    // printf("Read Result 2 \n");
    // for(int i = 0; i < 28; i++)
    //     printf("0x%02x\n", read_buffer[i]);

    printf("Done\n");
    return 0;
}