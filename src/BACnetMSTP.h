/**
 * @file BACnetMSTP.h
 * @brief BACnet MS/TP Arduino library — main entry-point header.
 * @author George Arun
 * @date 2025-2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * This is the only header the user includes in their sketch:
 *
 *   #include <BACnetMSTP.h>
 *
 *   BACnetMSTP bacnet(Serial1, 2);   // HardwareSerial port, DE/RE pin
 *
 *   void setup() { bacnet.begin(); }
 *   void loop()  { bacnet.task();  }
 *
 * All five sub-components (Device, AnalogValues, BinaryValues, Port, NVData)
 * are owned by value inside BACnetMSTP — no heap allocation is used.
 */

#ifndef BACNET_MSTP_H
#define BACNET_MSTP_H

#ifndef __cplusplus
#error "BACnetMSTP.h requires a C++ compiler"
#endif

#include "wrapper/BACnetDevice.h"
#include "wrapper/BACnetAnalogValue.h"
#include "wrapper/BACnetBinaryValue.h"
#include "wrapper/BACnetPort.h"
#include "wrapper/BACnetNVData.h"

class BACnetMSTP {                          /* KEYWORD1 */
public:
    /**
     * Constructor — serial port and DE/RE pin are mandatory.
     * The no-argument constructor is deleted because both values must be
     * known before begin() is called.
     *
     * @param serial   HardwareSerial port connected to the RS-485 transceiver.
     * @param derePin  Arduino pin number wired to the DE/RE line (active-HIGH).
     */
    BACnetMSTP(HardwareSerial &serial, uint8_t derePin);  /* KEYWORD1 */

    /** Deleted — serial port and DE/RE pin are mandatory at construction time. */
    BACnetMSTP() = delete;

    /** Non-copyable — C global state inside dlmstp/rs485 is not re-entrant. */
    BACnetMSTP(const BACnetMSTP&) = delete;
    BACnetMSTP& operator=(const BACnetMSTP&) = delete;

    /**
     * Initialise the entire BACnet stack.
     * Call once from Arduino setup().
     *
     * Initialisation order: NVData → Port → Device → AV → BV → DLMSTP →
     * mstimer → ADC.
     *
     * For BACNET_EEPROM_EXTERNAL_I2C: call Wire.begin() before begin().
     * For BACNET_EEPROM_FLASH_EMU:    EEPROM.begin() is called internally.
     *
     * @return true on success; false if EEPROM hardware fails or the RS-485
     *         port cannot initialise.
     */
    bool begin();                           /* KEYWORD2 */

    /**
     * Drive the BACnet MS/TP receive loop.
     * Call every Arduino loop() iteration as fast as possible.
     * Is a no-op if begin() was never called or returned false.
     */
    void task ();                           /* KEYWORD2 */

    /**
     * Trigger an unsolicited I-Am broadcast on the next task() call.
     * Useful after changing the device instance or on startup to announce
     * presence to the network without waiting for a Who-Is.
     */
    void sendIAm();                         /* KEYWORD2 */

    /** Direct access to the Device object for advanced configuration. */
    BACnetDevice      &device();            /* KEYWORD2 */

    /** Direct access to the Analog Value collection. */
    BACnetAnalogValue &analogValues();      /* KEYWORD2 */

    /** Direct access to the Binary Value collection. */
    BACnetBinaryValue &binaryValues();      /* KEYWORD2 */

    /** Direct access to the RS-485 port wrapper. */
    BACnetPort        &port();              /* KEYWORD2 */

    /** Direct access to the EEPROM data wrapper. */
    BACnetNVData      &nvdata();            /* KEYWORD2 */

private:
    BACnetPort        _port;
    BACnetNVData      _nvdata;
    BACnetDevice      _device;
    BACnetAnalogValue _av;
    BACnetBinaryValue _bv;

    bool _initialised;
};

#endif /* BACNET_MSTP_H */
