extern "C"
{
    #include "pico/stdio.h"
    #include "pico/stdlib.h"
    #include "hardware/uart.h"
    #include "driver/source/nmasic.h"
}

#include "WiFi101.h"

#define LED_PIN 25

void print_mac_address(uint8_t*);
void list_networks();
void print_encryption_type(int);

int main() {

    stdio_init_all();
    sleep_ms(1000*20); // Wait to get COM port setup
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    uint8_t led_val = 1;
    gpio_put(LED_PIN, led_val);

    printf("WiFi ATWINC1500 WiFi Chip Info:\n");
    printf("\tFrimware Version: %s\n", WiFi.firmwareVersion());
    uint8_t mac[6];
    WiFi.macAddress(mac);
    printf("\tMAC Address: ");
    print_mac_address(mac);

    sleep_ms(1000 * 10); // Give 10 second to read through info

    while( 1 )
    {
        printf("Scanning available networks");
        for(int i = 0; i < 5; i++ )
        {
            sleep_ms(950);
            printf(".");
        }
        printf("\n");
        list_networks();   
    }
    return 0;
}

void list_networks() {
  // scan for nearby networks:
  printf("** Scan Networks **\n");
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1)
  {
    printf("Couldn't get a WiFi connection\n");
    while (true);
  }

  // print the list of networks seen:
  printf("Number of available networks: %d\n", numSsid);

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    printf("%d) %s\tSignal: %d dBM\tEncryption: ", 
        thisNet, WiFi.SSID(thisNet), WiFi.RSSI(thisNet));
    print_encryption_type(WiFi.encryptionType(thisNet));
  }
}

void print_encryption_type(int thisType) {
  // read the encryption type and print out the name:
  switch (thisType) {
    case ENC_TYPE_WEP:
      printf("WEP\n");
      break;
    case ENC_TYPE_TKIP:
      printf("WPA\n");
      break;
    case ENC_TYPE_CCMP:
      printf("WPA2\n");
      break;
    case ENC_TYPE_NONE:
      printf("None\n");
      break;
    case ENC_TYPE_AUTO:
      printf("Auto\n");
      break;
  }
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