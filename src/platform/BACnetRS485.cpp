/**
 * @file BACnetRS485.cpp
 * @brief RS485 transceiver — Step 4, main lib implementation.
 *
 * STM32: HardwareSerial definitions MUST be at file scope here — before any
 * #include — so objects are constructed before Arduino framework init.
 * Putting them in a header causes ODR violations when included in multiple TUs.
 */

/* STM32 HardwareSerial definitions — file scope, before all includes */
#if defined(ARDUINO_NUCLEO_F756ZG)
#include <Arduino.h>
HardwareSerial Serial6(PG9, PG14);
#elif defined(ARDUINO_BLUEPILL_F103CB) || defined(ARDUINO_BLUEPILL_F103C8) || defined(ARDUINO_ARCH_STM32)
#include <Arduino.h>
HardwareSerial Serial2(PA3, PA2);   /* RX=PA3, TX=PA2 */
#endif

#include <string.h>
#include "BACnetRS485.h"
#include "rs485_port_config.h"

/* Single global instance — referenced by all extern "C" wrappers below */
static BACnetRS485 rs485;

/* -------------------------------------------------------------------------
   Constructor — no hardware access, runs before setup()
   ------------------------------------------------------------------------- */
BACnetRS485::BACnetRS485()
    : _baud(9600), _port(&RS485_PORT)
{
    memset(&_silenceTimer, 0, sizeof(_silenceTimer));
}

/* -------------------------------------------------------------------------
   begin() — configure DE/RE pin then open serial port
   ------------------------------------------------------------------------- */
void BACnetRS485::begin()
{
    BACnetHardware::pinInit(PIN_D8, true);
    BACnetHardware::pinWrite(PIN_D8, false);

#if defined(ARDUINO_ARCH_ESP32)
    _port->begin(_baud, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
#else
    _port->begin(_baud);
#endif
    timerSilenceReset();
}

/* -------------------------------------------------------------------------
   sendData() — assert DE/RE, write, flush, turnaround delay, release
   ------------------------------------------------------------------------- */
void BACnetRS485::sendData(const uint8_t *data, uint16_t length)
{
    assertDE();
    _port->write(data, length);
    _port->flush();
    bitTimeDelayAfterTx();
    releaseDE();
    timerSilenceReset();
}

/* -------------------------------------------------------------------------
   dataAvailable() — non-blocking single byte read
   ------------------------------------------------------------------------- */
bool BACnetRS485::dataAvailable(uint8_t *byte_out)
{
    if (_port->available() > 0) {
        *byte_out = (uint8_t)_port->read();
        return true;
    }
    return false;
}

/* -------------------------------------------------------------------------
   setBaudRate()
   ------------------------------------------------------------------------- */
bool BACnetRS485::setBaudRate(uint32_t baud)
{
    switch (baud) {
        case 9600: case 19200: case 38400:
        case 57600: case 76800: case 115200:
            _baud = baud;
            _port->end();
#if defined(ARDUINO_ARCH_ESP32)
            _port->begin(_baud, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
#else
            _port->begin(_baud);
#endif
            return true;
        default:
            return false;
    }
}

/* -------------------------------------------------------------------------
   baudFromKilo() — convert EEPROM baud_k byte to bps
   ------------------------------------------------------------------------- */
uint32_t BACnetRS485::baudFromKilo(uint8_t baud_k)
{
    if (baud_k == 255)      return 38400;
    if (baud_k >= 115)      return 115200;
    if (baud_k >= 76)       return 76800;
    if (baud_k >= 57)       return 57600;
    if (baud_k >= 38)       return 38400;
    if (baud_k >= 19)       return 19200;
    if (baud_k >= 9)        return 9600;
    return 38400;
}

/* -------------------------------------------------------------------------
   Silence timer
   ------------------------------------------------------------------------- */
unsigned long BACnetRS485::timerSilence()
{
    return mstimer_elapsed(&_silenceTimer);
}

void BACnetRS485::timerSilenceReset()
{
    mstimer_set(&_silenceTimer, 0);
}

/* -------------------------------------------------------------------------
   turnaroundDelay() — 40 bit-times per MS/TP spec
   ------------------------------------------------------------------------- */
void BACnetRS485::turnaroundDelay()
{
    uint32_t us = (40UL * 1000000UL) / _baud;
    delayMicroseconds((unsigned int)us);
}

/* -------------------------------------------------------------------------
   Private helpers
   ------------------------------------------------------------------------- */
void BACnetRS485::assertDE()
{
    BACnetHardware::pinWrite(PIN_D8, true);
    delayMicroseconds(1);
}

void BACnetRS485::releaseDE()
{
    BACnetHardware::pinWrite(PIN_D8, false);
}

void BACnetRS485::bitTimeDelayAfterTx()
{
    uint32_t us = (40UL * 1000000UL) / _baud;
    delayMicroseconds((unsigned int)us);
}

/* =========================================================================
   extern "C" wrappers — all symbols required by dlmstp.c and rs485.h
   ========================================================================= */
extern "C" {

void          RS485_Initialize(void)                    { rs485.begin(); }
void          RS485_Send_Data(const uint8_t *buf,
                              uint16_t n)               { rs485.sendData(buf, n); }
bool          RS485_DataAvailable(uint8_t *data)        { return rs485.dataAvailable(data); }
bool          RS485_ReceiveError(void)                  { return rs485.receiveError(); }
void          RS485_Transmitter_Enable(bool en)         { BACnetHardware::pinWrite(PIN_D8, en); }
void          RS485_Turnaround_Delay(void)              { rs485.turnaroundDelay(); }
void          RS485_LED_Timers(void)                    { rs485.ledTimers(); }
unsigned long RS485_Timer_Silence(void)                 { return rs485.timerSilence(); }
void          RS485_Timer_Silence_Reset(void)           { rs485.timerSilenceReset(); }
uint32_t      RS485_Get_Baud_Rate(void)                 { return rs485.getBaudRate(); }
bool          RS485_Set_Baud_Rate(uint32_t baud)        { return rs485.setBaudRate(baud); }
uint32_t      RS485_Baud_Rate_From_Kilo(uint8_t baud_k){ return BACnetRS485::baudFromKilo(baud_k); }

} /* extern "C" */
