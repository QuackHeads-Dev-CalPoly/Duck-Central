extern "C"
{
    #include "pico/stdio.h"
    #include "hardware/gpio.h"
}

#include "pwr_controller.h"

Pwr_Cntrl::Pwr_Cntrl()
{
    gpio_init(PERIPH_5V_PWR_EN); // Init
    gpio_pull_down(PERIPH_5V_PWR_EN); // Pull down the pin
    gpio_set_dir(PERIPH_5V_PWR_EN, true); // The pin to be output
    gpio_put(PERIPH_5V_PWR_EN, 0); // Keep it disabled

    gpio_init(GPS_PWR_EN); // Init
    gpio_pull_down(GPS_PWR_EN); // Pull down the pin to keep off
    gpio_set_dir(GPS_PWR_EN, true); // The pin to be output
    gpio_put(GPS_PWR_EN, 0); // Keep it disabled

    gpio_init(LORA_PWR_EN); // Init
    gpio_pull_down(LORA_PWR_EN); // Pull down the pin to keep off
    gpio_set_dir(LORA_PWR_EN, true); // The pin to be output
    gpio_put(LORA_PWR_EN, 0); // Keep it disabled

    gpio_init(WIFI_PWR_EN); // Init
    gpio_pull_down(WIFI_PWR_EN); // Pull down the pin to keep off
    gpio_set_dir(WIFI_PWR_EN, true); // The pin to be output
    gpio_put(WIFI_PWR_EN, 0); // Keep it disabled
}

int Pwr_Cntrl::turn_on_5v_pwr()
{
    gpio_put(PERIPH_5V_PWR_EN, 1); // Turn on the 5V boost
    pwr_state = pwr_state | PERIPH_5V_BIT_MASK; // Set memory state
    return 0;
}

int Pwr_Cntrl::turn_off_5v_pwr()
{
    gpio_put(PERIPH_5V_PWR_EN, 0); // Turn off the 5V boost
    pwr_state = pwr_state & ~(PERIPH_5V_BIT_MASK); // Set memory state
    return 0;
}

int Pwr_Cntrl::periph_5v_status()
{
    if((pwr_state & PERIPH_5V_BIT_MASK) == PERIPH_5V_BIT_MASK)
        return 1;
    else
        return 0;
}

int Pwr_Cntrl::turn_on_lora()
{
    gpio_put(LORA_PWR_EN, 1); // Turn on the LoRa radio
    pwr_state = pwr_state | LORA_PWR_BIT_MASK;
    return 0;
}

int Pwr_Cntrl::turn_off_lora()
{
    gpio_put(LORA_PWR_EN, 0); // Turn off the LoRa radio
    pwr_state = pwr_state & ~(LORA_PWR_BIT_MASK); //
    return 0;
}

int Pwr_Cntrl::lora_status()
{
    if((pwr_state & LORA_PWR_BIT_MASK) == LORA_PWR_BIT_MASK)
        return 1;
    else
        return 0;
}

int Pwr_Cntrl::turn_on_wifi()
{
    gpio_put(WIFI_PWR_EN, 1); // Turn on the WiFi radio
    pwr_state = pwr_state | WIFI_PWR_BIT_MASK;
    return 0;
}

int Pwr_Cntrl::turn_off_wifi()
{
    gpio_put(WIFI_PWR_EN, 0); // Turn off the WiFi radio
    pwr_state = pwr_state & ~(WIFI_PWR_BIT_MASK);
    return 0;
}

int Pwr_Cntrl::wifi_status()
{
    if((pwr_state & WIFI_PWR_BIT_MASK) == WIFI_PWR_BIT_MASK)
        return 1;
    else
        return 0;
}


