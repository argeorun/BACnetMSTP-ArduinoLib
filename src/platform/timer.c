/**
 * @file timer.c
 * @brief Arduino millis()-based BACnet mstimer HAL
 * @author George Arun
 * @date 2025-2026
 * @copyright SPDX-License-Identifier: MIT
 * @note Overrides the atmega328 hardware timer with millis().
 */
#include <Arduino.h>
#include "../bacnet/basic/sys/mstimer.h"

/* Nothing required for initialization when using millis(). */
void mstimer_init(void) {
  /* millis() starts automatically after boot */
}

/* Current time in milliseconds for BACnet stack timing. */
unsigned long mstimer_now(void) {
  return millis();
}

/* Legacy callback symbol retained (unused). */
void counter(void) {
  /* Intentionally empty - hardware interrupt not used in Arduino millis() version */
}
