#include <stdio.h>
#include <string.h>
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

/* ATWINC periph. LED, nums */
#define ERR_LED 18
#define WIFI_LED 16
#define NET_LED 15

static uint8 scan_request_index = 0;
static uint8 num_founded_ap = 0;

static void scan_wifi_cb(uint8_t u8MsgType, void *pvMsg)
{
    switch (u8MsgType)
    {
    case M2M_WIFI_RESP_SCAN_DONE:
    {
        tstrM2mScanDone *pstrInfo = (tstrM2mScanDone*) pvMsg;
        scan_request_index = 0;
        if(pstrInfo->u8NumofCh >= 1)
        {
            m2m_wifi_req_scan_result(scan_request_index);
            scan_request_index++;        
        }
        else
        {
            m2m_wifi_request_scan(M2M_WIFI_CH_ALL);
        }

        break;
    }

    case M2M_WIFI_RESP_SCAN_RESULT:
    {
        tstrM2mWifiscanResult *pstrScanResult = (tstrM2mWifiscanResult*) pvMsg;
        uint16_t scan_ssid_len = strlen((const char*)pstrScanResult->au8SSID);

        printf("[%d] SSID:%s\r\n", scan_request_index, pstrScanResult->au8SSID);

        num_founded_ap = m2m_wifi_get_num_ap_found();

        if( scan_request_index < num_founded_ap )
        {
            m2m_wifi_req_scan_result(scan_request_index);
            scan_request_index++;
        }
        else
        {
            sleep_ms(750);
            printf("---------------------------------\n");
            printf("---------------------------------\n");
            m2m_wifi_request_scan(M2M_WIFI_CH_ALL);
        }
    }
    default:
        break;
    }
    
}

void init_wifi_log_verion()
{
    tstrWifiInitParam param;
    nm_bsp_init();

    uint8 led_out = 1;
    gpio_put(LED_PIN, led_out);

    m2m_memset((uint8*) &param, 0, sizeof(param));
    param.pfAppWifiCb = scan_wifi_cb;

    sint8 ret = m2m_wifi_init(&param);
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
    tstrM2mRev strtmp;
    ret = nm_get_firmware_full_info(&strtmp);

    printf("Firmware: %d.%d.%d\n", strtmp.u8FirmwareMajor, 
                                    strtmp.u8FirmwareMinor,
                                    strtmp.u8FirmwarePatch);
    printf("Driver: %d.%d.%d\n", strtmp.u8DriverMajor,
                                    strtmp.u8DriverMinor,
                                    strtmp.u8DriverPatch );
    printf("Kevin chipID: %x\n", strtmp.u32Chipid);
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

            sleep_ms(500);
        }
    }

    while(1)
    {
        while(m2m_wifi_handle_events(NULL) != M2M_SUCCESS) 
        {
            printf("Failed to handle events\n");
        }
    }
}

int init_wifi_and_scan()
{
    tstrWifiInitParam param;
    nm_bsp_init();

    uint8 led_out = 1;
    gpio_put(LED_PIN, led_out);

    m2m_memset((uint8*) &param, 0, sizeof(param));
    param.pfAppWifiCb = scan_wifi_cb;

    sint8 ret = m2m_wifi_init(&param);
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
    tstrM2mRev strtmp;
    ret = nm_get_firmware_full_info(&strtmp);

    printf("Firmware: %d.%d.%d\n", strtmp.u8FirmwareMajor, 
                                    strtmp.u8FirmwareMinor,
                                    strtmp.u8FirmwarePatch);
    printf("Driver: %d.%d.%d\n", strtmp.u8DriverMajor,
                                    strtmp.u8DriverMinor,
                                    strtmp.u8DriverPatch );
    printf("Kevin chipID: %x\n", strtmp.u32Chipid);
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

            sleep_ms(500);
        }
    }

    ret = m2m_wifi_request_scan(M2M_WIFI_CH_ALL);
    if( M2M_SUCCESS != ret )
    {
        M2M_ERR("Scan request failed\n");
        while(1)
        {
            gpio_put(LED_PIN, led_out);
            if(!led_out)
                led_out++;
            else
                led_out--;

            sleep_ms(500);
        }
    }

    while(1)
    {
        while(m2m_wifi_handle_events(NULL) != M2M_SUCCESS) 
        {
            printf("Failed to handle events\n");
        }
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

int error_flash()
{
    uint8 led_out = 0;
    while(1)
    {
        gpio_put(LED_PIN, led_out);
        if(!led_out)
            led_out++;
        else
            led_out--;

        sleep_ms(500);
    }
}

static void ap_wifi_cb(uint8_t U8MsgType, void *pvMsg)
{
    switch(U8MsgType)
    {
        case M2M_WIFI_RESP_CON_STATE_CHANGED:
        {
            tstrM2mWifiStateChanged *pstrWifiState = (tstrM2mWifiStateChanged*) pvMsg;
            if(pstrWifiState->u8CurrState == M2M_WIFI_CONNECTED)
            {
                // Do nothing
            }
            else if(pstrWifiState->u8CurrState == M2M_WIFI_DISCONNECTED)
            {
                m2m_periph_gpio_set_val(M2M_PERIPH_GPIO4, 0);
                printf("Station disconnected\r\n");
            }

            break;
        }

        case M2M_WIFI_REQ_DHCP_CONF:
        {
            uint8_t *pu8IPAddress = (uint8_t *) pvMsg;
            printf("Station connected\r\n");
            printf("Station IP is %u.%u.%u.%u\r\n",
                    pu8IPAddress[0], pu8IPAddress[1], 
                    pu8IPAddress[2], pu8IPAddress[3]);
            m2m_periph_gpio_set_val(M2M_PERIPH_GPIO4, 1);
            break;

        }

        default:
        {
            break;
        }
    }

}

int init_wifi_create_ap()
{
    tstrWifiInitParam param;
    tstrM2MAPConfig strM2MAPConfig;

    sint8 ret;

    uint8 led_out = 1;
    gpio_put(LED_PIN, led_out);

    nm_bsp_init();
    memset((uint8 *) &param, 0, sizeof(tstrWifiInitParam));

    param.pfAppWifiCb = ap_wifi_cb;
    ret = m2m_wifi_init(&param);
    if(M2M_SUCCESS != ret)
    {
        M2M_ERR("Driver Init Failed <%d>\n", ret);
        error_flash();
    }
    
    tstrPerphInitParam periph_param;
    m2m_periph_init(&periph_param);

    m2m_periph_gpio_set_dir(M2M_PERIPH_GPIO5, 1);
    m2m_periph_gpio_set_dir(M2M_PERIPH_GPIO4, 1);
    m2m_periph_gpio_set_dir(M2M_PERIPH_GPIO15, 1);

    m2m_periph_gpio_set_val(M2M_PERIPH_GPIO5, 1);
    m2m_periph_gpio_set_val(M2M_PERIPH_GPIO4, 0);
    m2m_periph_gpio_set_val(M2M_PERIPH_GPIO15, 1);

    memset(&strM2MAPConfig, 0x00, sizeof(tstrM2MAPConfig));
    strcpy((char *)&strM2MAPConfig.au8SSID, "DUCK_AP");
    strM2MAPConfig.u8ListenChannel = (6);
    strM2MAPConfig.u8SecType = M2M_WIFI_SEC_OPEN;

    strM2MAPConfig.au8DHCPServerIP[0] = 192;
    strM2MAPConfig.au8DHCPServerIP[1] = 168;
    strM2MAPConfig.au8DHCPServerIP[2] = 1;
    strM2MAPConfig.au8DHCPServerIP[3] = 1;

    ret = m2m_wifi_enable_ap(&strM2MAPConfig);
    if( M2M_SUCCESS != ret )
    {
        M2M_ERR("Wifi enable ap failure <%d>\n", ret);
        error_flash();
    }

    printf("AP mode started\n");
    while(1)
    {
        while( m2m_wifi_handle_events(NULL) != M2M_SUCCESS)
        {
            printf("Handle event failure\n");
            error_flash();
        }
    }

    return 0;

}

int main() {

    stdio_init_all();
    sleep_ms(1000*30);
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    init_wifi_create_ap();

    while(1);

    return 0;
}
