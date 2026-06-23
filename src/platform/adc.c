/**
 * Arduino-compatible ADC wrapper
 * This file replaces AVR-specific ADC ISR and register usage with
 * Arduino IDE friendly calls (analogRead, noInterrupts/interrupts,
 * analogReference). It preserves the original adc API so existing
 * code can use `adc_result_8bit`, `adc_result_10bit`, `adc_millivolts`,
 * `adc_enable`, and `adc_init` but without AVR register access.
 */

#include <stdint.h>
#include <stdbool.h>
#include <Arduino.h>
#include "adc.h"
#include "pin_config.h"

#if defined(ARDUINO_ARCH_AVR)
#define ADC_VALUE_MAX 1024.0
#define ADC_MILLIVOLTS_MAX 5000.0

#elif defined(ARDUINO_ARCH_ESP32)
#define ADC_VALUE_MAX 4095.0
#define ADC_MILLIVOLTS_MAX 3300.0

#elif defined(ARDUINO_ARCH_STM32)
#define ADC_VALUE_MAX 4095.0
#define ADC_MILLIVOLTS_MAX 3300.0
#endif

static const uint8_t adc_pins[] = {
#ifdef PIN_A0
  PIN_A0,
#endif
#ifdef PIN_A1
  PIN_A1,
#endif
#ifdef PIN_A2
  PIN_A2,
#endif
#ifdef PIN_A3
  PIN_A3,
#endif
#ifdef PIN_A4
  PIN_A4,
#endif
#ifdef PIN_A5
  PIN_A5,
#endif
};

#define ADC_PIN_COUNT (sizeof(adc_pins) / sizeof(adc_pins[0]))
static volatile uint16_t Sample_Result[ADC_PIN_COUNT];
static volatile uint8_t Enabled_Channels;

/*
 * Note: The original AVR implementation used ADC interrupts and
 * hardware registers (ADCSRA, ADMUX, ISR(ADC_vect), etc.). On
 * Arduino (AVR core or other cores) the easiest portable approach
 * is to perform synchronous `analogRead()` calls. This file
 * provides the same API but performs immediate reads when a
 * channel is requested.
 */

void adc_init(void) {
  Enabled_Channels = 0;

#if defined(ARDUINO_ARCH_ESP32)|| defined(ARDUINO_ARCH_ESP32S3)
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

#elif defined(ARDUINO_ARCH_STM32)
  analogReadResolution(12);

#else
  // AVR (UNO, MEGA) → NO analogReadResolution
  // default is already 10-bit (0–1023)
#endif

  for (uint8_t i = 0; i < ADC_PIN_COUNT; i++) {
    Sample_Result[i] = 0;
  }
}

void adc_enable(uint8_t index) {
  if (index >= ADC_PIN_COUNT)
    return;
  /* Mark channel enabled and do an initial synchronous read to
       populate the sample result. Use noInterrupts/interrupts to
       mimic atomic behavior when updating shared state. */
  noInterrupts();
  Enabled_Channels |= (1 << index);
  /* analogRead takes either A0..An or 0..n depending on core.
       Passing the numeric index works on Arduino AVR and many
       other cores. If your platform requires `A0 + index`, change
       the call accordingly. */

  Sample_Result[index] = analogRead(adc_pins[index]);
  interrupts();
}

uint16_t adc_result_10bit(uint8_t index) {
  if (index >= ADC_PIN_COUNT)
    return 0;

  noInterrupts();

  Sample_Result[index] = analogRead(adc_pins[index]);  //  FIXED

  uint16_t result = Sample_Result[index];

  interrupts();

  return result;
}

uint8_t adc_result_8bit(uint8_t index) {
  uint16_t raw = adc_result_10bit(index);

#if defined(ARDUINO_ARCH_AVR)
  return (uint8_t)(raw >> 2);  // 10 → 8 bit

#else
  return (uint8_t)(raw >> 4);  // 12 → 8 bit
#endif
}


uint16_t adc_millivolts(uint8_t index) {
  float raw = adc_result_10bit(index);

  float mv = (raw / ADC_VALUE_MAX) * ADC_MILLIVOLTS_MAX;

  return (uint16_t)mv;
}
/*
 * The AVR code used an ISR and ADC registers to continuously sample
 * channels. That approach depends on direct register access and is
 * not portable to all Arduino cores or desirable for a generic
 * Arduino project. If you need interrupt-driven continuous sampling
 * on a specific board/architecture, implement a board-specific
 * variant that configures ADC registers or timers accordingly.
 */
