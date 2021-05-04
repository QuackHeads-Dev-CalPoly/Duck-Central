/*
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
#include "IridiumSBD.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


//--------- BEGIN CALLBACK FUNCTIONS
// not sure what this callback's purpose is - do not override
bool ISBDCallback() __attribute__((weak));
bool ISBDCallback() {
    // user would do stuff here...
    // then MUST return true or else you'll cancel every message
    return true;
}

// should probably be something like "print a character." flush and newlines not advised
void ISBDConsolePrintCallback(IridiumSBD *device, char c) __attribute__((weak));
void ISBDConsolePrintCallback(IridiumSBD *device, char c) {
    // default implementation does not print anything  
}

// noisier version of the ISBDConsolePrintCallback function
void ISBDDebugPrintCallback(IridiumSBD *device, char c) __attribute__((weak));
void ISBDDebugPrintCallback(IridiumSBD *device, char c) {
    // default implementation does not print anything
}

// get the milliseconds since system start
unsigned long ISBDMillis(IridiumSBD *device) __attribute__((weak));
unsigned long ISBDMillis() {
    return time_us_64() / 1000UL;
}
//--------- END CALLBACK FUNCTIONS


//--------- BEGIN PUBLIC INTERFACE

// Power on the RockBLOCK or return from sleep
int IridiumSBD::begin() {
    if (this->reentrant) return ISBD_REENTRANT;

    this->reentrant = true;
    int ret = internalBegin();
    this->reentrant = false;

    // Absent a successful startup, keep the device turned off
    if (ret != ISBD_SUCCESS) {
        power(false);
    }

    return ret;
}

// Transmit a binary message
int IridiumSBD::sendSBDBinary(const uint8_t *txData, size_t txDataSize) {
    if (this->reentrant) return ISBD_REENTRANT;

    this->reentrant = true;
    int ret = internalSendReceiveSBD(NULL, txData, txDataSize, NULL, NULL);
    this->reentrant = false;
    return ret;
}

// Transmit and receive a binary message
int IridiumSBD::sendReceiveSBDBinary(const uint8_t *txData, size_t txDataSize,
                                     uint8_t *rxBuffer, size_t &rxBufferSize) {
    if (this->reentrant) return ISBD_REENTRANT;

    this->reentrant = true;
    int ret = internalSendReceiveSBD(NULL, txData, txDataSize, rxBuffer,
                                     &rxBufferSize);
    this->reentrant = false;
    return ret;
}

// Transmit a text message
int IridiumSBD::sendSBDText(const char *message) {
    if (this->reentrant) return ISBD_REENTRANT;

    this->reentrant = true;
    int ret = internalSendReceiveSBD(message, NULL, 0, NULL, NULL);
    this->reentrant = false;
    return ret;
}

// Transmit a text message and receive reply
int IridiumSBD::sendReceiveSBDText(const char *message, uint8_t *rxBuffer,
                                   size_t &rxBufferSize) {
    if (this->reentrant) return ISBD_REENTRANT;

    this->reentrant = true;
    int ret = internalSendReceiveSBD(message, NULL, 0, rxBuffer, &rxBufferSize);
    this->reentrant = false;
    return ret;
}

// High-level wrapper for AT+CSQ
int IridiumSBD::getSignalQuality(int &quality) {
    if (this->reentrant) return ISBD_REENTRANT;

    this->reentrant = true;
    int ret = internalGetSignalQuality(quality);
    this->reentrant = false;
    return ret;
}

// Gracefully put device to lower power mode (if sleep pin provided)
int IridiumSBD::sleep() {
    if (this->reentrant) return ISBD_REENTRANT;

    if (this->sleepPin == -1) return ISBD_NO_SLEEP_PIN;

    this->reentrant = true;
    int ret = internalSleep();
    this->reentrant = false;

    if (ret == ISBD_SUCCESS) {
        power(false);  // power off
    }
    return ret;
}

// Return sleep state
bool IridiumSBD::isAsleep() { return this->asleep; }

// Return number of pending messages
int IridiumSBD::getWaitingMessageCount() { return this->remainingMessages; }

// Define capacitor recharge times
void IridiumSBD::setPowerProfile(
    POWERPROFILE profile)  // 0 = direct connect (default), 1 = USB
{
    switch (profile) {
        case DEFAULT_POWER_PROFILE:
            this->sbdixInterval = ISBD_DEFAULT_SBDIX_INTERVAL;
            break;

        case USB_POWER_PROFILE:
            this->sbdixInterval = ISBD_USB_SBDIX_INTERVAL;
            break;
    }
}

// Tweak AT timeout
void IridiumSBD::adjustATTimeout(int seconds) { 
    this->atTimeout = seconds;
}

// Tweak Send/Receive SBDIX process timeout
void IridiumSBD::adjustSendReceiveTimeout(int seconds) {
    this->sendReceiveTimeout = seconds;
}

// true to use workaround from Iridium Alert 5/7
void IridiumSBD::useMSSTMWorkaround(bool useWorkAround) {
    this->msstmWorkaroundRequested = useWorkAround;
}

// true to enable SBDRING alerts and RING signal pin
void IridiumSBD::enableRingAlerts(bool enable) {
    this->ringAlertsEnabled = enable;
    if (enable) {
        this->ringAsserted = false;
    }
}

bool IridiumSBD::hasRingAsserted() {
    if (!ringAlertsEnabled) return false;

    if (!reentrant) {
        // It's possible that the SBDRING message comes while we're not doing
        // anything
        filterSBDRING();
    }

    bool ret = ringAsserted;
    ringAsserted = false;
    return ret;
}

int IridiumSBD::getSystemTime(struct tm &tm) {
    char msstmResponseBuf[24];

    send("AT-MSSTM\r");
    if (!waitForATResponse(msstmResponseBuf, sizeof(msstmResponseBuf), "-MSSTM: ")) {
        return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
    }

    if (!isxdigit(msstmResponseBuf[0])) return ISBD_NO_NETWORK;

    // Latest epoch began at May 11, 2014, at 14:23:55 UTC.
    struct tm epoch_start;
    epoch_start.tm_year = 2014 - 1900;
    epoch_start.tm_mon = 5 - 1;
    epoch_start.tm_mday = 11;
    epoch_start.tm_hour = 14;
    epoch_start.tm_min = 23;
    epoch_start.tm_sec = 55;

    unsigned long ticks_since_epoch = strtoul(msstmResponseBuf, NULL, 16);

    /* Strategy: we'll convert to seconds by finding the largest number of
       integral seconds less than the equivalent ticks_since_epoch. Subtract
       that away and we'll be left with a small number that won't overflow when
       we scale by 90/1000.

       Many thanks to Scott Weldon for this suggestion.
    */
    unsigned long secs_since_epoch = (ticks_since_epoch / 1000) * 90;
    unsigned long small_ticks = ticks_since_epoch - (secs_since_epoch / 90) * 1000;
    secs_since_epoch += small_ticks * 90 / 1000;

    time_t epoch_time = mktime(&epoch_start);
    time_t now = epoch_time + secs_since_epoch;
    memcpy(&tm, localtime(&now), sizeof tm);
    return ISBD_SUCCESS;
}

int IridiumSBD::getFirmwareVersion(char *version, size_t bufferSize) {
    if (bufferSize < 8) return ISBD_RX_OVERFLOW;

    send("AT+CGMR\r");
    if (!waitForATResponse(version, bufferSize, "Call Processor Version: ")) {
        return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
    }

    return ISBD_SUCCESS;
}

//--------- END PUBLIC INTERFACE


//--------- BEGIN PRIVATE INTERFACE

int IridiumSBD::internalBegin() {
    debugprint("Calling internalBegin\r\n");

    if (!this->asleep) return ISBD_ALREADY_AWAKE;

    power(true);  // power on

    bool modemAlive = false;

    unsigned long startupTime = 500;  // ms
    for (unsigned long start = ISBDMillis(this); ISBDMillis(this) - start < startupTime;) {
        if (cancelled()) {
            return ISBD_CANCELLED;
        }
    }

    // Turn on modem and wait for a response from "AT" command to begin
    for (unsigned long start = ISBDMillis(this); !modemAlive && ISBDMillis(this) - start < 1000UL * ISBD_STARTUP_MAX_TIME;) {
        send("AT\r");
        modemAlive = waitForATResponse();
        if (cancelled()) return ISBD_CANCELLED;
    }

    if (!modemAlive) {
        debugprint("No modem detected.\r\n");
        return ISBD_NO_MODEM_DETECTED;
    }

    // The usual initialization sequence
    const char *strings[3] = {"ATE1\r", "AT&D0\r", "AT&K0\r"};
    for (int i = 0; i < 3; ++i) {
        send(strings[i]);
        if (!waitForATResponse()) {
            return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
        }
    }

    // Enable or disable RING alerts as requested by user
    // By default they are on if a RING pin was supplied on constructor
    debugprint("Ring alerts are");
    debugprint(ringAlertsEnabled ? "" : " NOT");
    debugprint(" enabled.\r\n");
    send(ringAlertsEnabled ? "AT+SBDMTA=1\r" : "AT+SBDMTA=0\r");
    if (!waitForATResponse()) {
        return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
    }

    // Decide whether the internal MSSTM workaround should be enforced on TX/RX
    // By default it is unless the firmware rev is >= TA13001
    char version[8];
    int ret = getFirmwareVersion(version, sizeof(version));
    if (ret != ISBD_SUCCESS) {
        debugprint("Unknown FW version\r\n");
        msstmWorkaroundRequested = true;
    } else {
        debugprint("Firmware version is ");
        debugprint(version);
        debugprint("\r\n");
        if (version[0] == 'T' && version[1] == 'A') {
            unsigned long ver = strtoul(version + 2, NULL, 10);
            msstmWorkaroundRequested = ver < ISBD_MSSTM_WORKAROUND_FW_VER;
        }
    }
    debugprint("MSSTM workaround is");
    debugprint(msstmWorkaroundRequested ? "" : " NOT");
    debugprint(" enforced.\r\n");

    // Done!
    debugprint("InternalBegin: success!\r\n");
    return ISBD_SUCCESS;
}

int IridiumSBD::internalSendReceiveSBD(const char *txTxtMessage,
                                       const uint8_t *txData, 
                                       size_t txDataSize,
                                       uint8_t *rxBuffer,
                                       size_t *prxBufferSize) {
    debugprint("internalSendReceive\r\n");

    if (this->asleep) return ISBD_IS_ASLEEP;

    // Binary transmission?
    if (txData && txDataSize) {
        if (txDataSize > ISBD_MAX_MESSAGE_LENGTH) return ISBD_MSG_TOO_LONG;

        send("AT+SBDWB=", true, false);
        send(txDataSize);
        send("\r", false);
        if (!waitForATResponse(NULL, 0, NULL, "READY\r\n")) {
            return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
        }

        uint16_t checksum = 0;
        for (size_t i = 0; i < txDataSize; ++i) {
            uart_putc_raw(uart_id, txData[i]);
            checksum += (uint16_t)txData[i];
        }

        consoleprint("[");
        consoleprint((uint16_t)txDataSize);
        consoleprint(" bytes]");

        debugprint("Checksum:");
        debugprint(checksum);
        debugprint("\r\n");

        uart_putc_raw(uart_id, checksum >> 8);
        uart_putc_raw(uart_id, checksum & 0xFF);

        if (!waitForATResponse(NULL, 0, NULL, "0\r\n\r\nOK\r\n")) {
            return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
        }
    }

    else  // Text transmission
    {
#if true                           // use long (340 character) string implementation
        if (txTxtMessage == NULL)  // It's ok to have a NULL txtTxtMessage if
                                   // the transaction is RX only
        {
            send("AT+SBDWT=\r");
            if (!waitForATResponse())
                return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
        } else {
            // remove any embedded \r
            char *p = strchr(txTxtMessage, '\r');
            if (p) *p = 0;
            if (strlen(txTxtMessage) > ISBD_MAX_MESSAGE_LENGTH)
                return ISBD_MSG_TOO_LONG;
            send("AT+SBDWT\r");
            if (!waitForATResponse(NULL, 0, NULL, "READY\r\n"))
                return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
            send(txTxtMessage);
            send("\r");
            if (!waitForATResponse(NULL, 0, NULL, "0\r\n\r\nOK\r\n"))
                return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
        }
#else  // use 120 character string implementation
        send("AT+SBDWT=", true, false);
        if (txTxtMessage != NULL)  // It's ok to have a NULL txtTxtMessage if
                                   // the transaction is RX only
            send(txTxtMessage);
        send("\r", false);
        if (!waitForATResponse())
            return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
#endif
    }

    // Long SBDIX loop begins here
    for (unsigned long start = ISBDMillis(this); ISBDMillis(this) - start < 1000UL * this->sendReceiveTimeout;) {
        bool okToProceed = true;
        if (this->msstmWorkaroundRequested) {
            okToProceed = false;
            int ret = internalMSSTMWorkaround(okToProceed);
            if (ret != ISBD_SUCCESS) return ret;
        }

        if (okToProceed) {
            uint16_t moCode = 0, moMSN = 0, mtCode = 0, mtMSN = 0, mtLen = 0, mtRemaining = 0;
            int ret = doSBDIX(moCode, moMSN, mtCode, mtMSN, mtLen, mtRemaining);
            if (ret != ISBD_SUCCESS) return ret;

            debugprint("SBDIX MO code: ");
            debugprint(moCode);
            debugprint("\r\n");

            // this range indicates successful return!
            if (moCode >= 0 && moCode <= 4) {
                debugprint("SBDIX success!\r\n");

                this->remainingMessages = mtRemaining;
                if (mtCode == 1 && rxBuffer) {
                    // retrieved 1 message
                    debugprint("Incoming message!\r\n");
                    return doSBDRB(rxBuffer, prxBufferSize);
                }

                else {
                    // No data returned
                    if (prxBufferSize) *prxBufferSize = 0;
                }
                return ISBD_SUCCESS;
            }
            // fatal failure: no retry
            else if (moCode == 12 || moCode == 14 || moCode == 16) {
                debugprint("SBDIX fatal!\r\n");
                return ISBD_SBDIX_FATAL_ERROR;
            }
            // retry
            else {
                debugprint("Waiting for SBDIX retry...\r\n");
                if (!noBlockWait(sbdixInterval)) return ISBD_CANCELLED;
            }
        }
        // MSSTM check fail
        else {
            debugprint("Waiting for MSSTM retry...\r\n");
            if (!noBlockWait(ISBD_MSSTM_RETRY_INTERVAL)) return ISBD_CANCELLED;
        }
    }  // big wait loop

    debugprint("SBDIX timeout!\r\n");
    return ISBD_SENDRECEIVE_TIMEOUT;
}

int IridiumSBD::internalGetSignalQuality(int &quality) {
    if (this->asleep) return ISBD_IS_ASLEEP;

    char csqResponseBuf[2];

    send("AT+CSQ\r");
    if (!waitForATResponse(csqResponseBuf, sizeof(csqResponseBuf), "+CSQ:")) {
        return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
    }

    if (isdigit(csqResponseBuf[0])) {
        quality = atoi(csqResponseBuf);
        return ISBD_SUCCESS;
    }

    return ISBD_PROTOCOL_ERROR;
}

int IridiumSBD::internalMSSTMWorkaround(bool &okToProceed) {
    /*
    According to Iridium 9602 Product Bulletin of 7 May 2013, to overcome a
    system erratum:

    "Before attempting any of the following commands: +SBDDET, +SBDREG, +SBDI,
    +SBDIX, +SBDIXA the field application should issue the AT command AT-MSSTM
    to the transceiver and evaluate the response to determine if it is valid or
    not:

    Valid Response: "-MSSTM: XXXXXXXX" where XXXXXXXX is an eight-digit
    hexadecimal number.

    Invalid Response: "-MSSTM: no network service"

    If the response is invalid, the field application should wait and recheck
    system time until a valid response is obtained before proceeding.

    This will ensure that the Iridium SBD transceiver has received a valid
    system time before attempting SBD communication. The Iridium SBD transceiver
    will receive the valid system time from the Iridium network when it has a
    good link to the satellite. Ensuring that the received signal strength
    reported in response to AT command +CSQ and +CIER is above 2-3 bars before
    attempting SBD communication will protect against lockout.
    */
    char msstmResponseBuf[24];

    send("AT-MSSTM\r");
    if (!waitForATResponse(msstmResponseBuf, sizeof(msstmResponseBuf), "-MSSTM: ")) {
        return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
    }

    // Response buf now contains either an 8-digit number or the string "no
    // network service"
    okToProceed = isxdigit(msstmResponseBuf[0]);
    return ISBD_SUCCESS;
}

int IridiumSBD::internalSleep() {
    if (this->asleep) return ISBD_IS_ASLEEP;
    return ISBD_SUCCESS;
}

bool IridiumSBD::noBlockWait(int seconds) {
    for (unsigned long start = ISBDMillis(this); ISBDMillis(this) - start < 1000UL * seconds;) {
        if (cancelled()) return false;
    }

    return true;
}

// Wait for response from previous AT command.  This process terminates when
// "terminator" string is seen or upon timeout. If "prompt" string is provided
// (example "+CSQ:"), then all characters following prompt up to the next CRLF
// are stored in response buffer for later parsing by caller.
bool IridiumSBD::waitForATResponse(char *response, int responseSize,
                                   const char *prompt, const char *terminator) {
    debugprint("Waiting for response ");
    debugprint(terminator);
    debugprint("\r\n");

    if (response) {
        memset(response, 0, responseSize);
    }

    int matchPromptPos = 0;      // Matches chars in prompt
    int matchTerminatorPos = 0;  // Matches chars in terminator
    enum { LOOKING_FOR_PROMPT, GATHERING_RESPONSE, LOOKING_FOR_TERMINATOR };
    int promptState = prompt ? LOOKING_FOR_PROMPT : LOOKING_FOR_TERMINATOR;

    debugprint("Timeout is ");
    debugprint(atTimeout);
    debugprint(" seconds.\n");

    consoleprint("<< ");
    for (unsigned long start = ISBDMillis(this); ISBDMillis(this) - start < 1000UL * atTimeout;) {
        if (cancelled()) {
            return false;
        }

        while (filteredavailable() > 0) {
            char c = filteredread();
            if (prompt) {
                switch (promptState) {
                    case LOOKING_FOR_PROMPT:
                        if (c == prompt[matchPromptPos]) {
                            ++matchPromptPos;
                            if (prompt[matchPromptPos] == '\0')
                                promptState = GATHERING_RESPONSE;
                        }

                        else {
                            matchPromptPos = c == prompt[0] ? 1 : 0;
                        }

                        break;
                    case GATHERING_RESPONSE:  // gathering response from end of
                                              // prompt to first \r
                        if (response) {
                            if (c == '\r' || responseSize < 2) {
                                promptState = LOOKING_FOR_TERMINATOR;
                            } else {
                                *response++ = c;
                                responseSize--;
                            }
                        }
                        break;
                }
            }

            if (c == terminator[matchTerminatorPos]) {
                ++matchTerminatorPos;
                if (terminator[matchTerminatorPos] == '\0') return true;
            } else {
                matchTerminatorPos = c == terminator[0] ? 1 : 0;
            }
        }  // while (uart_is_readable(uart_id) > 0)
    }      // timer loop
    debugprint("waitForATResponse timed out.\n");
    return false;
}

bool IridiumSBD::cancelled() {
    //if (ringPin != -1 && digitalRead(ringPin) == LOW)  // Active low per guide
        //ringAsserted = true;

    return !ISBDCallback();
}

int IridiumSBD::doSBDIX(uint16_t &moCode, uint16_t &moMSN, uint16_t &mtCode,
                        uint16_t &mtMSN, uint16_t &mtLen,
                        uint16_t &mtRemaining) {
    // Returns xx,xxxxx,xx,xxxxx,xx,xxx
    char sbdixResponseBuf[32];
    send("AT+SBDIX\r");
    if (!waitForATResponse(sbdixResponseBuf, sizeof(sbdixResponseBuf),
                           "+SBDIX: "))
        return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;

    uint16_t *values[6] = {&moCode, &moMSN, &mtCode,
                           &mtMSN,  &mtLen, &mtRemaining};
    for (int i = 0; i < 6; ++i) {
        char *p = strtok(i == 0 ? sbdixResponseBuf : NULL, ", ");
        if (p == NULL) return ISBD_PROTOCOL_ERROR;
        *values[i] = atol(p);
    }
    return ISBD_SUCCESS;
}

int IridiumSBD::doSBDRB(uint8_t *rxBuffer, size_t *prxBufferSize) {
    bool rxOverflow = false;

    send("AT+SBDRB\r");
    if (!waitForATResponse(NULL, 0, NULL,
                           "AT+SBDRB\r"))  // waits for its own echo
        return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;

    // Time to read the binary data: size[2], body[size], checksum[2]
    unsigned long start = ISBDMillis(this);
    while (ISBDMillis(this) - start < 1000UL * atTimeout) {
        if (cancelled()) return ISBD_CANCELLED;
        if (uart_is_readable(uart_id) >= 2) break;
    }

    if (uart_is_readable(uart_id) < 2) return ISBD_SENDRECEIVE_TIMEOUT;

    uint16_t size = 256 * uart_getc(uart_id) + uart_getc(uart_id);
    consoleprint("[Binary size:");
    consoleprint(size);
    consoleprint("]");

    for (uint16_t bytesRead = 0; bytesRead < size;) {
        if (cancelled()) return ISBD_CANCELLED;

        if (uart_is_readable(uart_id)) {
            uint8_t c = uart_getc(uart_id);
            bytesRead++;
            if (rxBuffer && prxBufferSize) {
                if (*prxBufferSize > 0) {
                    *rxBuffer++ = c;
                    (*prxBufferSize)--;
                } else {
                    rxOverflow = true;
                }
            }
        }

        if (ISBDMillis(this) - start >= 1000UL * atTimeout)
            return ISBD_SENDRECEIVE_TIMEOUT;
    }

    while (ISBDMillis(this) - start < 1000UL * atTimeout) {
        if (cancelled()) return ISBD_CANCELLED;
        if (uart_is_readable(uart_id) >= 2) break;
    }

    if (uart_is_readable(uart_id) < 2) return ISBD_SENDRECEIVE_TIMEOUT;

    uint16_t checksum = 256 * uart_getc(uart_id) + uart_getc(uart_id);
    consoleprint("[csum:");
    consoleprint(checksum);
    consoleprint("]");

    // Return actual size of returned buffer
    if (prxBufferSize) *prxBufferSize = (size_t)size;

    // Wait for final OK
    if (!waitForATResponse())
        return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;

    return rxOverflow ? ISBD_RX_OVERFLOW : ISBD_SUCCESS;
}

void IridiumSBD::power(bool on) {
    this->asleep = !on;

    if (this->sleepPin == -1) return;

    // pinMode(this->sleepPin, OUTPUT);

    if (on) {
        debugprint("Powering on modem...\r\n");
        //digitalWrite(this->sleepPin, HIGH);  // HIGH = awake
        lastPowerOnTime = ISBDMillis(this);
    }

    else {
        // Best Practices Guide suggests waiting at least 2 seconds
        // before powering off again
        unsigned long elapsed = ISBDMillis(this) - lastPowerOnTime;
        //if (elapsed < 2000UL) delay(2000UL - elapsed);

        debugprint("Powering off modem...\r\n");
        //digitalWrite(this->sleepPin, LOW);  // LOW = asleep
    }
}

void IridiumSBD::send(std::string str, bool beginLine, bool endLine) {
    if (beginLine) consoleprint(">> ");
    consoleprint(str);
    if (endLine) consoleprint("\r\n");
    uart_write_blocking(uart_id, (const uint8_t *)str.c_str(), (size_t)str.length());
}

void IridiumSBD::send(const char *str) {
    consoleprint(">> ");
    consoleprint(str);
    consoleprint("\r\n");

    char c;
    int i = 0;
    while ((c = str[i++]) != '\0') {
        uart_putc_raw(uart_id, c);
    }
}

void IridiumSBD::send(uint16_t n) {
    consoleprint(n);
    char buf[2];
    sprintf(buf, "%d", n);
    uart_write_blocking(uart_id, (const uint8_t*) buf, 2);
}

void IridiumSBD::debugprint(std::string str) {
    for (char &c : str) {
        ISBDDebugPrintCallback(this, c);
    }
    fflush(stdout);
}

void IridiumSBD::debugprint(const char *str) {
    while (*str) ISBDDebugPrintCallback(this, *str++);
    fflush(stdout);
}

void IridiumSBD::debugprint(uint16_t n) {
    char str[10];
    sprintf(str, "%u", n);
    debugprint(str);
    fflush(stdout);
}

void IridiumSBD::consoleprint(std::string str) {
    printf("%s\n", str.c_str());
    fflush(stdout);
}

void IridiumSBD::consoleprint(const char *str) {
    while (*str) ISBDConsolePrintCallback(this, *str++);
    fflush(stdout);
}

void IridiumSBD::consoleprint(uint16_t n) {
    char str[10];
    sprintf(str, "%u", n);
    consoleprint(str);
    fflush(stdout);
}

void IridiumSBD::consoleprint(char c) { 
    ISBDConsolePrintCallback(this, c);
    fflush(stdout);
}

void IridiumSBD::SBDRINGSeen() {
    ringAsserted = true;
    debugprint("SBDRING alert seen!\r\n");
}

// Read characters until we find one that doesn't match SBDRING
// If nextChar is -1 it means we are still entertaining a possible
// match with SBDRING\r\n.  Once we find a mismatch, stuff it into
// nextChar.
void IridiumSBD::filterSBDRING() {
    while (uart_is_readable(uart_id) > 0 && nextChar == -1) {
        char c = uart_getc(uart_id);
        consoleprint(c);
        if (*head != 0 && c == *head) {
            ++head;
            if (*head == 0) {
                SBDRINGSeen();
                head = tail = SBDRING;
            } else {
                // Delay no more than 10 milliseconds waiting for next char in
                // SBDRING
                for (unsigned long start = ISBDMillis(this);
                     uart_is_readable(uart_id) == 0 &&
                     ISBDMillis(this) - start < FILTERTIMEOUT;)
                    ;

                // If there isn't one, assume this ISN'T an unsolicited SBDRING
                if (uart_is_readable(uart_id) ==
                    0)  // pop the character back into nextChar
                {
                    --head;
                    nextChar = c;
                }
            }
        } else {
            nextChar = c;
        }
    }
}

const char IridiumSBD::SBDRING[] = "SBDRING\r\n";

int IridiumSBD::filteredavailable() {
    filterSBDRING();
    return head - tail + (nextChar != -1 ? 1 : 0);
}

int IridiumSBD::filteredread() {
    filterSBDRING();

    // Use up the queue first
    if (head > tail) {
        char c = *tail++;
        if (head == tail) head = tail = SBDRING;
        return c;
    }

    // Then the "extra" char
    else if (nextChar != -1) {
        char c = (char)nextChar;
        nextChar = -1;
        return c;
    }

    return -1;
}

//--------- END PRIVATE INTERFACE