/**
 * @file BACnetStack.cpp
 * @brief BACnetMSTP class implementation — top-level library compositor.
 * @author George Arun
 * @date 2025-2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * This file owns the BACnetMSTP::begin() initialisation sequence and the
 * BACnetMSTP::task() MS/TP receive loop.
 *
 * Include order rule: compile_config.h MUST be first — use the path without
 * "../" because this file lives in src/ (not src/wrapper/).
 */

#include "compile_config.h"     /* MUST be first — no ../ from src/ root */

#if !defined(ARDUINO)
#error "BACnetStack.cpp is Arduino-only; do not include it in desktop builds."
#endif

#include <Arduino.h>            /* interrupts(), noInterrupts(), delay(), millis() */
#include "BACnetMSTP.h"

extern "C" {
#include "bacnet/datalink/dlmstp.h"         /* dlmstp_init, dlmstp_receive, dlmstp_set_* */
#include "bacnet/basic/npdu/h_npdu.h"       /* npdu_handler */
#include "bacnet/basic/sys/mstimer.h"       /* mstimer_init */
#include "platform/adc.h"                   /* adc_init */
}

/* Catch platforms where float is not 32-bit (catches exotic DSP toolchains) */
static_assert(sizeof(float) == 4, "float must be 4 bytes on this platform");

/* =========================================================================
 * PDU receive buffer — shared across task() calls.
 * MAX_PDU is defined in bacnet/bacdef.h (included transitively via dlmstp.h).
 * Static at file scope avoids stack overflow on processors with limited RAM.
 * ========================================================================= */
static uint8_t s_pdu_buf[MAX_PDU];

/* =========================================================================
 * Constructor
 * ========================================================================= */

BACnetMSTP::BACnetMSTP(HardwareSerial &serial, uint8_t derePin)
    : _port(serial, derePin)
    , _nvdata()
    , _device()
    , _av()
    , _bv()
    , _initialised(false)
{
}

/* =========================================================================
 * begin() — call once from setup()
 *
 * Initialisation order (must be preserved):
 *   1. NVData      — EEPROM backend init + magic-marker validation;
 *                    writes factory defaults on first boot.
 *   2. Port        — RS-485 serial port + DE/RE pin; baud from EEPROM.
 *   3. Device      — load device instance number and name strings from EEPROM.
 *   4. AV / BV     — initialise C object arrays and configure pin modes.
 *   5. DLMSTP      — configure MAC / baud / max-master, then dlmstp_init().
 *   6. mstimer     — Arduino millisecond tick (must follow serial init).
 *   7. ADC         — enable analogue input channels.
 * ========================================================================= */

bool BACnetMSTP::begin()
{
    if (_initialised) {
        return false;   /* duplicate-init guard — C global state is not re-entrant */
    }

    /* ---- 1. NVData ---------------------------------------------------- */
    if (!_nvdata.begin()) {
        /* First boot or EEPROM schema mismatch — write factory defaults.
         * If the hardware itself fails (I2C chip absent) the second
         * _nvdata.begin() call will also return false and we abort.        */
        nvdata_unsigned16_set(NV_EEPROM_TYPE_0,          NV_EEPROM_TYPE_ID);
        nvdata_unsigned8_set (NV_EEPROM_VERSION,          NV_EEPROM_VERSION_ID);
        nvdata_unsigned8_set (NV_EEPROM_MSTP_MAC,         10);   /* MAC 10      */
        nvdata_unsigned8_set (NV_EEPROM_MSTP_BAUD_K,      38);   /* 38.4 kbaud */
        nvdata_unsigned8_set (NV_EEPROM_MSTP_MAX_MASTER,  127);
        nvdata_unsigned24_set(NV_EEPROM_DEVICE_0, BACNET_DEVICE_INSTANCE_DEFAULT);

        if (!_nvdata.begin()) {
            return false;   /* real hardware failure */
        }
    }

    /* ---- 2. Port ------------------------------------------------------- */
    uint8_t  baud_k = nvdata_unsigned8(NV_EEPROM_MSTP_BAUD_K);
    uint32_t baud   = RS485_Baud_Rate_From_Kilo(baud_k);

    if (!_port.begin(baud)) {
        return false;
    }

    /* ---- 3. Device identity ------------------------------------------- */
    _device.begin(_nvdata);

    /* ---- 4. Object arrays --------------------------------------------- */
    Analog_Value_Init();
    Binary_Value_Init();

    /* ---- 5. DLMSTP ----------------------------------------------------- */
    dlmstp_set_mac_address  (nvdata_unsigned8(NV_EEPROM_MSTP_MAC));
    dlmstp_set_max_master   (nvdata_unsigned8(NV_EEPROM_MSTP_MAX_MASTER));
    /* Note: baud rate is applied by _port.begin() via RS485_Set_Baud_Rate().
     * dlmstp_set_baud_rate() is not implemented in the platform override;
     * the RS-485 hardware init in Port::begin() already handles baud.     */
    dlmstp_set_max_info_frames(1);
    dlmstp_init(NULL);

    /* ---- 6. Millisecond timer ----------------------------------------- */
    mstimer_init();

    /* ---- 7. ADC -------------------------------------------------------- */
    adc_init();

#if defined(ARDUINO)
    interrupts();
#endif

    _initialised = true;
    return true;
}

/* =========================================================================
 * task() — call every loop() iteration
 *
 * Drives:
 *   a) _port.task()   → RS485_LED_Timers() (physical layer)
 *   b) dlmstp_receive → one MPDU per iteration (non-blocking, timeout = 0)
 *   c) npdu_handler   → NPDU dispatch → apdu_handler → handler_read_property /
 *                        handler_write_property / handler_who_is
 * ========================================================================= */

void BACnetMSTP::task()
{
    if (!_initialised) {
        return;
    }

    /* Physical layer — RS-485 LED timers */
    _port.task();

    /* Protocol layer — receive one MPDU and dispatch it */
    BACNET_ADDRESS src;
    uint16_t pdu_len = dlmstp_receive(&src, s_pdu_buf, sizeof(s_pdu_buf), 0);
    if (pdu_len > 0) {
        npdu_handler(&src, s_pdu_buf, pdu_len);
    }
}

/* =========================================================================
 * Sub-component accessors
 * ========================================================================= */

BACnetDevice      &BACnetMSTP::device()       { return _device; }
BACnetAnalogValue &BACnetMSTP::analogValues() { return _av; }
BACnetBinaryValue &BACnetMSTP::binaryValues() { return _bv; }
BACnetPort        &BACnetMSTP::port()         { return _port; }
BACnetNVData      &BACnetMSTP::nvdata()       { return _nvdata; }
