#ifndef __POWER_CONTROL_H__
#define __POWER_CONTROL_H__

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

class PowerControl
{

    private: 
        uint8_t pwr_state;

    public:
        PowerControl();

        int turn_off_gps();
        int turn_on_gps();
        bool gps_enabled();

        int turn_off_lora();
        int turn_on_lora();
        bool lora_enabled();

        int turn_off_wifi();
        int turn_on_wifi();
        bool wifi_enabled();

        int turn_off_5v_pwr();
        int turn_on_5v_pwr();
        bool periph_5v_enabled();
};


#endif /* Power Controller */