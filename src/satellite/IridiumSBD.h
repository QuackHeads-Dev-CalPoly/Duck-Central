/*
IridiumSBD - An Arduino library for Iridium SBD ("Short Burst Data")
Communications Suggested and generously supported by Rock Seven Location
Technology (http://rock7mobile.com), makers of the brilliant RockBLOCK satellite
modem. Copyright (C) 2013-2017 Mikal Hart All rights reserved.

The latest version of this library is available at http://arduiniana.org.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#define ISBD_LIBRARY_REVISION 2
#define ISBD_DEFAULT_AT_TIMEOUT 30  //seconds
#define ISBD_MSSTM_RETRY_INTERVAL 10  //seconds
#define ISBD_DEFAULT_SBDIX_INTERVAL 10  //seconds
#define ISBD_USB_SBDIX_INTERVAL 30  //seconds
#define ISBD_DEFAULT_SENDRECEIVE_TIME 300 //seconds
#define ISBD_STARTUP_MAX_TIME 240  //seconds
#define ISBD_MAX_MESSAGE_LENGTH 340  //characters
#define ISBD_MSSTM_WORKAROUND_FW_VER 13001

#define ISBD_SUCCESS 0
#define ISBD_ALREADY_AWAKE 1
#define ISBD_SERIAL_FAILURE 2
#define ISBD_PROTOCOL_ERROR 3
#define ISBD_CANCELLED 4
#define ISBD_NO_MODEM_DETECTED 5
#define ISBD_SBDIX_FATAL_ERROR 6
#define ISBD_SENDRECEIVE_TIMEOUT 7
#define ISBD_RX_OVERFLOW 8
#define ISBD_REENTRANT 9
#define ISBD_IS_ASLEEP 10
#define ISBD_NO_SLEEP_PIN 11
#define ISBD_NO_NETWORK 12
#define ISBD_MSG_TOO_LONG 13

#include <string>
#include "pico/stdlib.h"
#include "hardware/uart.h"

// gets millis since board was turned on
static unsigned long millis();

class IridiumSBD {
   public:
    int begin();
    int sendSBDText(const char *message);
    int sendSBDBinary(const uint8_t *txData, size_t txDataSize);
    int sendReceiveSBDText(const char *message, uint8_t *rxBuffer,
                           size_t &rxBufferSize);
    int sendReceiveSBDBinary(const uint8_t *txData, size_t txDataSize,
                             uint8_t *rxBuffer, size_t &rxBufferSize);
    int getSignalQuality(int &quality);
    int getSystemTime(struct tm &tm);
    int getFirmwareVersion(char *version, size_t bufferSize);
    int getWaitingMessageCount();
    bool isAsleep();
    bool hasRingAsserted();
    int sleep();

    typedef enum {
        DEFAULT_POWER_PROFILE = 0,
        USB_POWER_PROFILE = 1
    } POWERPROFILE;

    void setPowerProfile(POWERPROFILE profile);  // 0 = direct connect (default), 1 = USB
    void adjustATTimeout(int seconds);  // default value = 20 seconds
    void adjustSendReceiveTimeout(int seconds);  // default value = 300 seconds
    void useMSSTMWorkaround(bool useMSSTMWorkAround);  // true to use workaround from Iridium Alert
    void enableRingAlerts(bool enable);

    IridiumSBD(uart_inst_t *const uart_num, int buad, int txPinNo, int rxPinNo, int sleepPinNo = -1, int ringPinNo = -1): 
        uart_id(uart_num),
        sbdixInterval(ISBD_USB_SBDIX_INTERVAL),
        atTimeout(ISBD_DEFAULT_AT_TIMEOUT),
        sendReceiveTimeout(ISBD_DEFAULT_SENDRECEIVE_TIME),
        remainingMessages(-1),
        asleep(true),
        reentrant(false),
        sleepPin(sleepPinNo),
        ringPin(ringPinNo),
        msstmWorkaroundRequested(true),
        ringAlertsEnabled(ringPinNo != -1),
        ringAsserted(false),
        lastPowerOnTime(0UL),
        head(SBDRING),
        tail(SBDRING),
        nextChar(-1) 
    {
        // if (sleepPin != -1) pinMode(sleepPin, OUTPUT);
        // if (ringPin != -1) pinMode(ringPin, INPUT);
        uart_init(uart_id, buad);
        gpio_set_function(rxPinNo, GPIO_FUNC_UART);
        gpio_set_function(txPinNo, GPIO_FUNC_UART);
    }

   private:
       uart_inst_t *const uart_id;

       // Timings
       int sbdixInterval;
       int atTimeout;
       int sendReceiveTimeout;

       // State variables
       int remainingMessages;
       bool asleep;
       bool reentrant;
       int sleepPin;
       int ringPin;
       bool msstmWorkaroundRequested;
       bool ringAlertsEnabled;
       bool ringAsserted;
       unsigned long lastPowerOnTime;

       // Internal utilities
       bool noBlockWait(int seconds);
       bool waitForATResponse(char *response = NULL, int responseSize = 0,
                              const char *prompt = NULL,
                              const char *terminator = "OK\r\n");

       int internalBegin();
       int internalSendReceiveSBD(const char *txTxtMessage, 
                                  const uint8_t *txData,
                                  size_t txDataSize, 
                                  uint8_t *rxBuffer,
                                  size_t *prxBufferSize);
       int internalGetSignalQuality(int &quality);
       int internalMSSTMWorkaround(bool &okToProceed);
       int internalSleep();

       int doSBDIX(uint16_t &moCode, uint16_t &moMSN, uint16_t &mtCode,
                   uint16_t &mtMSN, uint16_t &mtLen, uint16_t &mtRemaining);
       int doSBDRB(uint8_t *rxBuffer, size_t *prxBufferSize); // in/out
       void power(bool on);

       void send(std::string str, bool beginLine = true, bool endLine = true);
       void send(const char *str);
       void send(uint16_t n);

       bool cancelled(); // call ISBDCallback and see if client cancelled the operation

       void debugprint(std::string str);
       void debugprint(const char *str);
       void debugprint(uint16_t n);

       void consoleprint(std::string str);
       void consoleprint(const char *str);
       void consoleprint(uint16_t n);
       void consoleprint(char c);
       void SBDRINGSeen();

       // Unsolicited SBDRING filter technology
       static const char SBDRING[];
       static const int FILTERTIMEOUT = 10;
       const char *head, *tail;
       int nextChar;
       void filterSBDRING();
       int filteredavailable();
       int filteredread();
};