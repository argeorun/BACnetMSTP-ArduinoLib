/**
 * @file BACnetPort.h
 * @brief RS-485 / MS-TP datalink port wrapper.
 * @author George Arun
 * @date 2025-2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * Wraps src/platform/rs485.cpp and the RS-485 configuration so that the
 * hardware serial port and DE/RE pin can be selected at runtime rather than
 * requiring a rebuild with different rs485_port_config.h settings.
 *
 * All timing and framing stay inside the existing dlmstp.c / rs485.cpp files;
 * this class only configures and drives them.
 */

#ifndef BACNET_PORT_H
#define BACNET_PORT_H

#ifndef __cplusplus
#error "BACnetPort.h requires a C++ compiler"
#endif

#include "../compile_config.h"  /* must be first */
#include <stdint.h>
#include <stdbool.h>
#include <HardwareSerial.h>

/* rs485.h manages its own extern "C" guards for pure-C functions.
 * RS485_Set_Serial_Port is declared OUTSIDE those guards (C++ linkage, takes
 * HardwareSerial*). Including inside an outer extern "C" block would force C
 * linkage on that declaration and break the link against rs485.cpp. */
#include "../platform/rs485.h"

extern "C" {
/* No additional C headers needed here for BACnetPort. */
}

class BACnetPort {
public:
    /**
     * Constructor — the serial port reference and DE/RE pin are mandatory.
     * The default no-arg constructor is deleted because both values must be
     * known before begin() is called.
     *
     * @param serial   HardwareSerial port connected to the RS-485 transceiver.
     * @param derePin  Arduino pin number wired to the DE/RE lines (active-HIGH).
     */
    BACnetPort(HardwareSerial &serial, uint8_t derePin);
    BACnetPort() = delete;

    /**
     * Initialise the RS-485 port at the requested baud rate.
     * Calls RS485_Set_Enable_Pin(), RS485_Initialize(), RS485_Set_Baud_Rate().
     * Must be called once from BACnetMSTP::begin().
     *
     * @param baud   MS/TP baud rate (9600 / 19200 / 38400 / 57600 / 76800 / 115200).
     * @return true if the serial port initialised and the baud rate is valid.
     */
    bool     begin      (uint32_t baud);

    /** Change the baud rate after begin(). */
    bool     setBaud    (uint32_t baud);

    /** @return the current baud rate. */
    uint32_t getBaud    () const;

    /** Change the DE/RE pin assignment (must be called before begin()). */
    void     setDEREPin (uint8_t pin);

    /** @return the current DE/RE pin number. */
    uint8_t  getDEREPin () const;

    /**
     * Called from BACnetMSTP::task() every loop iteration.
     * Drives the MS/TP state machine inside dlmstp.c.
     */
    void     task       ();

    /* --- Non-copyable --- */
    BACnetPort(const BACnetPort&)            = delete;
    BACnetPort& operator=(const BACnetPort&) = delete;

private:
    HardwareSerial &_serial;
    uint8_t         _derePin;
    uint32_t        _baud;
};

#endif /* BACNET_PORT_H */
