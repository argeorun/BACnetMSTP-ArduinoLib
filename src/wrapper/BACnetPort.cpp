/**
 * @file BACnetPort.cpp
 * @brief RS-485 / MS-TP datalink port wrapper — implementation.
 * @author George Arun
 * @date 2025-2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * Arduino-only translation unit. Delegates all low-level RS-485 work to
 * rs485.cpp / dlmstp.c via their public C APIs. No direct register
 * manipulation lives here.
 */

#if !defined(ARDUINO)
#error "BACnetPort.cpp is Arduino-only; do not include it in desktop builds."
#endif

#include "BACnetPort.h"

extern "C" {
#include "../bacnet/datalink/dlmstp.h"  /* DLMSTP_Cleanup / DLMSTP_Receive */
}

/* =========================================================================
 * Constructor
 * ========================================================================= */

BACnetPort::BACnetPort(HardwareSerial &serial, uint8_t derePin)
    : _serial(serial)
    , _derePin(derePin)
    , _baud(9600)
{
}

/* =========================================================================
 * begin() — configure and start the RS-485 port
 * ========================================================================= */

bool BACnetPort::begin(uint32_t baud)
{
    /* Push the user's chosen serial port and DE/RE pin into rs485.cpp
     * before RS485_Initialize() reads them.                              */
    RS485_Set_Serial_Port(&_serial);
    RS485_Set_Enable_Pin(_derePin);

    RS485_Initialize();

    if (!RS485_Set_Baud_Rate(baud)) {
        return false;
    }
    _baud = baud;
    return true;
}

/* =========================================================================
 * setBaud() / getBaud()
 * ========================================================================= */

bool BACnetPort::setBaud(uint32_t baud)
{
    if (!RS485_Set_Baud_Rate(baud)) {
        return false;
    }
    _baud = baud;
    return true;
}

uint32_t BACnetPort::getBaud() const
{
    return _baud;
}

/* =========================================================================
 * setDEREPin() / getDEREPin()
 * ========================================================================= */

void BACnetPort::setDEREPin(uint8_t pin)
{
    _derePin = pin;
    RS485_Set_Enable_Pin(pin);
}

uint8_t BACnetPort::getDEREPin() const
{
    return _derePin;
}

/* =========================================================================
 * task() — called from BACnetMSTP::task() every loop iteration
 * ========================================================================= */

void BACnetPort::task()
{
    /* The MS/TP state machine is driven via DLMSTP_Receive in BACnetStack.cpp
     * (where the receive buffer and service dispatch also live).
     * BACnetPort::task() drives LED timers and other periodic port work.  */
    RS485_LED_Timers();
}
