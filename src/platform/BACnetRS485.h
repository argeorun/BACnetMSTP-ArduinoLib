/**
 * @file BACnetRS485.h
 * @brief RS485 transceiver class — Step 4, main lib.
 *
 * Wraps the board-specific HardwareSerial port and DE/RE control pin.
 * Provides all symbols required by dlmstp.c via extern "C" wrappers
 * in BACnetRS485.cpp.
 *
 * ODR note: BACnetRS485.h does NOT include rs485_port_config.h.
 *           The port macro (RS485_PORT) is used only in BACnetRS485.cpp.
 */

#ifndef BACNET_RS485_H
#define BACNET_RS485_H

#include <stdint.h>
#include <stdbool.h>

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "BACnetHardware.h"
#include "../bacnet/basic/sys/mstimer.h"

class BACnetRS485 {
public:
    BACnetRS485();

    void     begin();
    void     sendData(const uint8_t *data, uint16_t length);
    bool     dataAvailable(uint8_t *byte_out);
    bool     receiveError() const { return false; }
    void     ledTimers() {}
    void     turnaroundDelay();

    uint32_t getBaudRate() const { return _baud; }
    bool     setBaudRate(uint32_t baud);

    unsigned long timerSilence();
    void          timerSilenceReset();

    static uint32_t baudFromKilo(uint8_t baud_k);

private:
    uint32_t           _baud;
    HardwareSerial    *_port;
    struct mstimer     _silenceTimer;

    void assertDE();
    void releaseDE();
    void bitTimeDelayAfterTx();
};

#endif /* BACNET_RS485_H */
