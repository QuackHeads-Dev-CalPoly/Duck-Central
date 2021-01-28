#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"

#include "bsp/include/nm_bsp.h"
#include "bsp/include/nm_bsp_rp2040.h"
#include "driver/include/m2m_wifi.h"
#include "driver/include/m2m_periph.h"
#include "driver/include/m2m_ssl.h"
#include "common/include/nm_common.h"
#include "driver/source/nmasic.h"

#define LED_PIN 25

static void wifi_cb(uint8_t u8MsgType, void *pvMsg)
{
    printf("Here in Wifi callback\n");
    
}

void init_wifi()
{
    tstrWifiInitParam param;
    nm_bsp_init();

    uint8 led_out = 1;
    gpio_put(LED_PIN, led_out);

    m2m_memset((uint8*) &param, 0, sizeof(param));
    param.pfAppWifiCb = wifi_cb;

    sint8 ret = m2m_wifi_init(&param);
    uint32 chip_id = nmi_get_chipid();
    printf("Kevin chipID: %d", chip_id);
    sleep_ms(1000);
    if( M2M_SUCCESS != ret )
    {
        M2M_ERR("Driver Init Failed <%d>\n", ret);
        while(1)
        {
            gpio_put(LED_PIN, led_out);
            if(!led_out)
                led_out++;
            else
                led_out--;
            //printf("Driver Init failed | ret: %d\n", ret);

            sleep_ms(1000);
        }
    }

    while(1)
    {
        while(m2m_wifi_handle_events(NULL) != M2M_SUCCESS) 
        {
            printf("Failed to handle events\n");
        }
        printf("In while loop\n");
    }

}

int blink()
{
    stdio_init_all();
        
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while(1)
    {
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
        gpio_put(LED_PIN, 1);
        printf("Hello World, from Pico");
        printf("Size of: %ld\n", sizeof(size_t));
        sleep_ms(1000);
    }
}

int main() {

    stdio_init_all();
    sleep_ms(1000*30);
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    init_wifi();

    while(1);

    return 0;
}
