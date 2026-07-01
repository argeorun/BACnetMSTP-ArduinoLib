/**
 * @file BACnetMSTP-ArduinoLib.ino
 * @brief Arduino BACnet MS/TP device — example sketch
 * @author George Arun
 * @date 2026
 * @copyright SPDX-License-Identifier: MIT
 */

#include "compile_config.h"

#if defined(ARDUINO)
#include <Arduino.h>
#endif

/* ESP32/ESP32-S3: increase loopTask stack. Must be after Arduino.h. */
#if defined(ARDUINO_ARCH_ESP32)
SET_LOOP_TASK_STACK_SIZE(16384);
#endif

#include "BACnetMSTP.h"

BACnetDevice device;

void setup() {
    device.begin();
}

void loop() {
    device.update();
}
