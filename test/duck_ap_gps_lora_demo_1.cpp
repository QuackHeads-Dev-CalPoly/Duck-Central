extern "C"
{
    #include "pico/stdio.h"
    #include "pico/stdlib.h"
    #include "hardware/uart.h"
    #include "driver/source/nmasic.h"
    #include "driver/include/m2m_periph.h"
    #include "lora.h"
    #include "batt_pwr_and_temp_sensor.h"
    #include "index.h"
}

#include "WiFi101.h"

/*
    Make sure wiring is correct 
*/

void print_mac_address(uint8_t*);

#define LED_PIN 25

int status = WL_IDLE_STATUS;
WiFiServer server(80);

int main() {

    stdio_init_all();
    sleep_ms(1000*20);
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    uint8_t led_val = 1;
    gpio_put(LED_PIN, led_val);

    // Init ADC temp and battery library ( Only for quacker board )
    init_batt_pwr_and_temp_sensor();

    printf("Setting up lora...\n");
    lora_setup(LOGGING_LIGHT);
    sleep_ms(1000);
    char msg_setup[] = "LoRa radio setup.";
    char msg_low[] = "GET /L. fshore!";
    char msg_high[] = "GET /H. fshore!";
    printf("Lora setup!\n");
    lora_send_packet((uint8_t*) msg_setup, strlen(msg_setup));
    sleep_ms(500);

    printf("Creating access point named: QUACK NET\n");

    status = WiFi.beginAP("QUACK NET");
    if(status != WL_AP_LISTENING)
    {
        printf("Creating access point failed\n");
        while(1)
        {
            gpio_put(LED_PIN, led_val);
            sleep_ms(1000);
            (led_val > 0) ? led_val = 0 : led_val = 1;
        }
    }

    sleep_ms(1000 * 10); // Wait for 10 seconds for AP creation

    server.begin();
    printf("SSID: %s | Local IP (Reversed and HEX): %x\n", WiFi.SSID(), WiFi.localIP());

    char http_header[]
        = "HTTP/1.1 200 OK\n";
    char content_header[]
        = "Content-type: text/html; charset=UTF-8\n";

    char content_1[]
        = "Click <a href=\"/H\">here</a> turn the LED on pin 9 on<br>";
    char content_2[]
        =  "Click <a href=\"/L\">here</a> turn the LED on pin 9 off<br>";

    while( 1 )
    {
        if(status != WiFi.status()) // Status changed
        {
            status = WiFi.status();

            if(status == WL_AP_CONNECTED)
            {
                uint8_t remote_mac[6];
                WiFi.APClientMacAddress(remote_mac);

                printf("Device connected to AP, MAC address: ");
                m2m_periph_gpio_set_val(M2M_PERIPH_GPIO4, 0);
                print_mac_address(remote_mac);
            }
            else
            {
                server.begin(); // Added this. Everytime a client disconnected the server needs to restart
                printf("Device disconnected from AP\n");
            }
        }

        WiFiClient client = server.available(); // listen for incoming clients

        if(client)
        {
            printf("new client\n");
            char currLine[255];
            uint8_t curr_char = 0;
            while(client.connected())
            {
                if(client.available())
                {
                    char c = client.read();
                    //printf("%c\n", c);
                    if( c == '\n' )
                    {
                        if(curr_char == 0)
                        {
                            
                            // /* Header */
                            client.write((uint8_t*) &http_header, (size_t) 16);
                            client.write('\n');
                            
                            client.write((uint8_t*) MAIN_page, (size_t) 5170);
                            client.write('\n');

                            break;
                        } else {
                            memset(&currLine, '\0', 255);
                            curr_char = 0;
                        }
                    } else if(c != '\r') {
                        currLine[curr_char++] = c;
                    }

                    if(currLine[0] == 'G' && currLine[1] == 'E' &&
                        currLine[2] == 'T' && currLine[3] == ' ' &&
                        currLine[4] == '/' && currLine[5] == 'L')
                    {
                        gpio_put(LED_PIN, 0);
                        lora_send_packet((uint8_t*) msg_low, strlen(msg_low));
                        printf("VBUS: %f\n", get_bus_voltage_float());
                        printf("VBAT: %f\n", get_batt_voltage_float());
                    }

                    if(currLine[0] == 'G' && currLine[1] == 'E' &&
                        currLine[2] == 'T' && currLine[3] == ' ' &&
                        currLine[4] == '/' && currLine[5] == 'H')
                    {
                        gpio_put(LED_PIN, 1);
                        lora_send_packet((uint8_t*) msg_high, strlen(msg_high));
                        printf("VBUS: %f\n", get_bus_voltage_float());
                        printf("VBAT: %f\n", get_batt_voltage_float());
                    }

                }
            }
            client.stop();
            printf("client disconnected\n");
        }
    }
    return 0;
}

void print_mac_address(uint8_t* mac) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      printf("0");
    }
    printf("%x", mac[i]);
    if (i > 0) {
      printf(":");
    }
  }
  printf("\n");
}