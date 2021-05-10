#ifndef __POWER_CONTROLLER_H__
#define __POWER_CONTROLLER_H__

extern "C"
{
    #include <stdint.h>
}

// GP Pin definitions for power defintions. ONLY on REV 0.3
#define GPS_PWR_EN 0
#define GPS_PWR_BIT_MASK 0x08

#define LORA_PWR_EN 2
#define LORA_PWR_BIT_MASK 0x04

#define WIFI_PWR_EN 7
#define WIFI_PWR_BIT_MASK 0x02

#define PERIPH_5V_PWR_EN 28
#define PERIPH_5V_BIT_MASK 0x01

class Pwr_Cntrl
{

    private: 
        uint8_t pwr_state;

    public:
        Pwr_Cntrl();

        int turn_off_gps();
        int turn_on_gps();
        int gps_status();

        int turn_off_lora();
        int turn_on_lora();
        int lora_status();

        int turn_off_wifi();
        int turn_on_wifi();
        int wifi_status();

        int turn_off_5v_pwr();
        int turn_on_5v_pwr();
        int periph_5v_status();
};


#endif /* PWR Controller */