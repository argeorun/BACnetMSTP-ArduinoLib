/**
 * @file rs485_port_config.h
 * @brief RS485 serial port selection — declarations only (no definitions).
 *
 * HardwareSerial object definitions live in BACnetRS485.cpp at file scope.
 * This header provides extern declarations and port macros only — ODR safe.
 */

#ifndef RS485_PORT_CONFIG_H
#define RS485_PORT_CONFIG_H

#ifdef ARDUINO
#include <Arduino.h>
#endif

#if defined(ARDUINO_NUCLEO_F756ZG)
    extern HardwareSerial Serial6;
    #define RS485_PORT      Serial6
    #define RS485_PORT_NAME "Serial6 (PG9/PG14)"

#elif defined(ARDUINO_BLUEPILL_F103CB) || defined(ARDUINO_BLUEPILL_F103C8) || defined(ARDUINO_ARCH_STM32)
    /* Blue Pill default: Serial2 (PA2/PA3). To use Serial3 (PB10/PB11) change here. */
    extern HardwareSerial Serial2;
    #define RS485_PORT      Serial2
    #define RS485_PORT_NAME "Serial2 (PA2/PA3)"

#elif defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_IDF_TARGET_ESP32S3)
    #define RS485_PORT      Serial2
    #define RS485_RX_PIN    16
    #define RS485_TX_PIN    17
    #define RS485_PORT_NAME "Serial2 (GPIO16/17)"

#elif defined(ARDUINO_ARCH_ESP32)
    #define RS485_PORT      Serial2
    #define RS485_RX_PIN    16
    #define RS485_TX_PIN    17
    #define RS485_PORT_NAME "Serial2 (GPIO16/17)"

#elif defined(ARDUINO_AVR_MEGA2560)
    #define RS485_PORT      Serial1
    #define RS485_PORT_NAME "Serial1 (TX1/RX1)"

#elif defined(ARDUINO_AVR_UNO)
    #define RS485_PORT      Serial
    #define RS485_PORT_NAME "Serial (shared)"

#else
#  error "Unsupported board — add RS485 port mapping here"
#endif

#endif /* RS485_PORT_CONFIG_H */
