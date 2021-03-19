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

int mess_send_flag = 0;
int mess_cpy_flag = 0;
char mess_line[255];
int led_off_flag = 0;
int led_on_flag = 0;
int bat_volt_flag = 0;

int status = WL_IDLE_STATUS;
WiFiServer server(80);

int main() {

    stdio_init_all();
    sleep_ms(1000*20);
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    uint8_t led_val = 1;
    gpio_put(LED_PIN, led_val);

    memset(&mess_line, '\0', 255);
    printf("%s", MAIN_page);

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

    //sleep_ms(1000 * 10); // Wait for 10 seconds for AP creation

    server.begin();
    printf("SSID: %s | Local IP (Reversed and HEX): %x\n", WiFi.SSID(), WiFi.localIP());

    char http_header[]
        = "HTTP/1.1 200 OK\n";
    char content_header[]
        = "Content-type: text/html; charset=UTF-8\n";


    char content_1[]
        = "Click <a href=\"/H\">here</a> turn the LED on<br>";
    char content_2[]
        = "Click <a href=\"/L\">here</a> turn the LED off<br>";
    
    char content_3[]
        = "Click <a href=\"/bat\">here</a> to send battery voltage<br>";

    char form_content[]
        = "<!DOCTYPE html><html><head><title>ClusterDuck Protocol</title></head><body><h2 class=\"\">You are connected to a ClusterDuck</h2><h3>Send message below</h3><div><form action=\"/send\" method=\"GET\" enctype=\"text/plain\"><input id=\"message\" type=\"text\" name=\"message\" /><input type=\"submit\"/></form></div><div>Click <a href=\"/bat\">here</a> to send battery voltage<br></div><div>Click <a href=\"/L\">here</a> turn the LED off<br></div><div>Click <a href=\"/H\">here</a> turn the LED on<br></div></body></html>";

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

        if( mess_send_flag )
        {
            printf("Send message\n");
            printf("Message line: %s\n", mess_line);
            char lora_message[255];
            int i;
            int j = 0;
            for( i = 18; i < 255; i++ )
            {
                if( mess_line[i] == '+' )
                    lora_message[j++] = ' ';
                else if ( mess_line[i] == ' ' ) 
                {
                    lora_message[j] = '\0'; // End message string
                    break;
                }
                else
                    lora_message[j++] = mess_line[i];
            }
            printf("Lora Message: %s\n", lora_message);
            lora_send_packet((uint8_t *) lora_message, j);
            mess_send_flag = 0;
            mess_cpy_flag = 0;
        }

        if( led_off_flag ) 
        {
            gpio_put(LED_PIN, 0);
            lora_send_packet((uint8_t*) msg_low, strlen(msg_low));
            led_off_flag = 0;
        }

        if( led_on_flag )
        {
            gpio_put(LED_PIN, 1);
            lora_send_packet((uint8_t*) msg_high, strlen(msg_high));
            led_on_flag = 0;
        }

        if( bat_volt_flag )
        {
            char charged[] = "Battery is charged";
            char charging[] = "Battery is charging";
            char not_full[] = "Battery not full";
            if(get_batt_voltage_float() > 4.1)
                lora_send_packet((uint8_t*) charged, 19);
            if(get_batt_voltage_float() < 4.1 && get_bus_voltage_float() > 4.0)
                lora_send_packet((uint8_t*) charging, 20);
            else
                lora_send_packet((uint8_t*) not_full, 17);

            bat_volt_flag = 0;
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
                            client.write((uint8_t*) &content_header, (size_t) 39);
                            client.write('\n');

                            client.write((uint8_t*) &MAIN_page, (size_t) 1400);
                            client.write((uint8_t*) &MAIN_page[1400], (size_t) 1400);
                            client.write((uint8_t*) &MAIN_page[2800], (size_t) 1400);
                            client.write((uint8_t*) &MAIN_page[4200], (size_t) 563);

                            break;
                        } else {
                            printf("C %s\n", currLine);
                            if(mess_send_flag && !mess_cpy_flag)
                            {
                                memcpy(&mess_line, &currLine, 255);
                                mess_cpy_flag = 1;
                            }
                            memset(&currLine, '\0', 255);
                            curr_char = 0;
                        }
                    } else if(c != '\r') {
                        currLine[curr_char++] = c;
                    }

                    if(currLine[0] == 'G' && currLine[1] == 'E' &&
                        currLine[2] == 'T' && currLine[3] == ' ' &&
                        currLine[4] == '/' && currLine[5] == 's' &&
                        currLine[6] == 'e' && currLine[7] == 'n' &&
                        currLine[8] == 'd')
                    {
                        mess_send_flag = 1;
                    }

                    if(currLine[0] == 'G' && currLine[1] == 'E' &&
                        currLine[2] == 'T' && currLine[3] == ' ' &&
                        currLine[4] == '/' && currLine[5] == 'L')
                    {
                        led_off_flag = 1;
                    }

                    if(currLine[0] == 'G' && currLine[1] == 'E' &&
                        currLine[2] == 'T' && currLine[3] == ' ' &&
                        currLine[4] == '/' && currLine[5] == 'H')
                    {
                        led_on_flag = 1;
                    }

                    if(currLine[0] == 'G' && currLine[1] == 'E' &&
                        currLine[2] == 'T' && currLine[3] == ' ' && 
                        currLine[4] == '/' && currLine[5] == 'b' &&
                        currLine[6] == 'a' && currLine[7] == 't')
                    {
                        bat_volt_flag = 1;
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