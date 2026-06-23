/**
 * @file pin_config.c
 * @brief Board-specific pin assignments and I/O initialisation
 * @author George Arun
 * @date 2025-2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * Implements the PIN_INIT / PIN_WRITE / PIN_READ API declared in pin_config.h.
 * Handles ESP32-S3's RGB built-in LED transparently.
 */
#include "pin_config.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif


void PIN_INIT(uint8_t pin, bool is_output)
{
#ifdef ARDUINO
#if defined(RGB_BUILTIN)
    /* ESP32-S3 RGB LED is driven via rgbLedWrite(); it does not use pinMode. */
    if (pin == RGB_BUILTIN) {
        return;
    }
#endif
    pinMode(pin, is_output ? OUTPUT : INPUT);

    if (is_output) {
        digitalWrite(pin, LOW);
    }
#endif
}


void PIN_WRITE(uint8_t pin, bool active)
{
#ifdef ARDUINO
#if defined(RGB_BUILTIN)
    /* Handle ESP32-S3 onboard RGB LED */
    if (pin == RGB_BUILTIN) {
        rgbLedWrite(RGB_BUILTIN,
                    active ? 64 : 0,
                    active ? 64 : 0,
                    active ? 64 : 0);
        return;
    }
#endif
    digitalWrite(pin, active ? HIGH : LOW);
#endif
}


bool PIN_READ(uint8_t pin)
{
#ifdef ARDUINO
#if defined(RGB_BUILTIN)
    /* RGB LED has no readable state */
    if (pin == RGB_BUILTIN) {
        return false;
    }
#endif
    return (digitalRead(pin) == HIGH);
#else
    (void)pin;
    return false;
#endif
}
