/**
 * @file rs485.cpp
 * @brief Arduino RS-485 port implementation of the bacnet-stack rs485.h API
 * @author George Arun
 * @date 2025-2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * Based on BACnetMSTP-Arduino-R1/src/platform/rs485.cpp.
 * Extended with RS485_Set_Serial_Port() and RS485_Set_Enable_Pin() so the
 * BACnetMSTP library can inject the user's chosen HardwareSerial port and
 * DE/RE pin at runtime (called by BACnetPort::begin() before RS485_Initialize()).
 *
 * Default port  : RS485_PORT   (from rs485_port_config.h)
 * Default DE/RE : PIN_D8       (from pin_config.h)
 */
#include <Arduino.h>
#include "../bacnet/basic/sys/mstimer.h"
#include "rs485.h"
#include "pin_config.h"
#include "rs485_port_config.h"

/* =========================================================================
 * Runtime-configurable state
 * Defaults match the compile-time constants so the library works even
 * if RS485_Set_Serial_Port / RS485_Set_Enable_Pin are never called.
 * ========================================================================= */
static HardwareSerial *RS485_Serial     = &RS485_PORT;
static uint8_t         RS485_Enable_Pin = PIN_D8;

static uint32_t        RS485_Baud       = 9600;
static struct mstimer  Silence_Timer    = { 0, 0 };

/* =========================================================================
 * Runtime overrides — called by BACnetPort::begin()
 * ========================================================================= */

void RS485_Set_Serial_Port(HardwareSerial *port)
{
    if (port) {
        RS485_Serial = port;
    }
}

void RS485_Set_Enable_Pin(uint8_t pin)
{
    RS485_Enable_Pin = pin;
}

/* =========================================================================
 * Silence timer
 * ========================================================================= */

unsigned long RS485_Timer_Silence(void)
{
    return mstimer_elapsed(&Silence_Timer);
}

void RS485_Timer_Silence_Reset(void)
{
    mstimer_set(&Silence_Timer, 0);
}

/* =========================================================================
 * Initialisation
 * ========================================================================= */

void RS485_Initialize(void)
{
    pinMode(RS485_Enable_Pin, OUTPUT);
    RS485_Transmitter_Enable(false);    /* start in receive mode */

#if defined(ARDUINO_ARCH_ESP32)
    RS485_Serial->begin(RS485_Baud, SERIAL_8N1, PIN_UART2_RX, PIN_UART2_TX);
#else
    RS485_Serial->begin(RS485_Baud);
#endif

    RS485_Timer_Silence_Reset();
}

/* =========================================================================
 * Baud rate helpers
 * ========================================================================= */

uint32_t RS485_Get_Baud_Rate(void)
{
    return RS485_Baud;
}

uint32_t RS485_Baud_Rate_From_Kilo(uint8_t baud_k)
{
    uint32_t baud;

    if (baud_k == 255) {
        baud = 38400;
    } else if (baud_k >= 115) {
        baud = 115200;
    } else if (baud_k >= 76) {
        baud = 76800;
    } else if (baud_k >= 57) {
        baud = 57600;
    } else if (baud_k >= 38) {
        baud = 38400;
    } else if (baud_k >= 19) {
        baud = 19200;
    } else if (baud_k >= 9) {
        baud = 9600;
    } else {
        baud = 38400;
    }

    return baud;
}

bool RS485_Set_Baud_Rate(uint32_t baud)
{
    switch (baud) {
        case 9600:
        case 19200:
        case 38400:
        case 57600:
        case 76800:
        case 115200:
            RS485_Baud = baud;
            RS485_Serial->end();
#if defined(ARDUINO_ARCH_ESP32)
            RS485_Serial->begin(RS485_Baud, SERIAL_8N1, PIN_UART2_RX, PIN_UART2_TX);
#else
            RS485_Serial->begin(RS485_Baud);
#endif
            return true;
        default:
            return false;
    }
}

/* =========================================================================
 * Transmitter enable / data send / receive
 * ========================================================================= */

void RS485_Transmitter_Enable(bool enable)
{
    digitalWrite(RS485_Enable_Pin, enable ? HIGH : LOW);
}

void RS485_Turnaround_Delay(void)
{
    /* 40 bit times per BACnet MS/TP spec */
    unsigned long us = (40UL * 1000000UL) / RS485_Baud;
    delayMicroseconds(us);
}

void RS485_Send_Data(const uint8_t *buffer, uint16_t nbytes)
{
    RS485_Transmitter_Enable(true);
    RS485_Serial->write(buffer, nbytes);
    RS485_Serial->flush();          /* wait until transmit FIFO empty */
    RS485_Turnaround_Delay();
    RS485_Transmitter_Enable(false);
    RS485_Timer_Silence_Reset();
}

bool RS485_ReceiveError(void)
{
    /* Arduino HardwareSerial does not expose framing/overrun status. */
    return false;
}

bool RS485_DataAvailable(uint8_t *data)
{
    if (RS485_Serial->available() > 0) {
        *data = static_cast<uint8_t>(RS485_Serial->read());
        return true;
    }
    return false;
}

void RS485_LED_Timers(void)
{
    /* Optional: add LED activity indication here if required. */
}
