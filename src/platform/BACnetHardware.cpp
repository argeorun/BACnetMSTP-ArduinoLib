/**
 * @file BACnetHardware.cpp
 * @brief Platform pin, ADC, and stack monitoring — implementation
 * @author George Arun
 * @date 2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * Replaces: pin_config.c, adc.c, stack.c (all deleted in Step 2).
 *
 * Provides two parallel APIs:
 *   1. C++ static class methods (BACnetHardware::pinInit etc.) for .ino and
 *      future C++ app classes (BACnetAV, BACnetBV).
 *   2. extern "C" wrappers with the old C function names (PIN_INIT, adc_millivolts,
 *      stack_unused etc.) so av.c and bv.c continue to link without change.
 *
 * AVR stack canary note:
 *   stack_init() on AVR must remain a free extern "C" function with
 *   __attribute__((naked)) __attribute__((section(".init1"))). It cannot be a
 *   class method — GCC section placement attributes only work on free functions.
 *   The class method BACnetHardware::stackInit() is an empty stub on AVR; the
 *   real fill runs automatically from .init1 before main().
 */

#include "BACnetHardware.h"

/* =========================================================================
   AVR stack canary — free function, must stay here, cannot be a class method.
   Placed in .init1 so it runs before main(), before any C++ constructors.
   ========================================================================= */
#if defined(ARDUINO_ARCH_AVR) && defined(__GNUC__)

extern uint8_t _end;
extern uint8_t __stack;

#define STACK_CANARY (0xC5)

extern "C" void stack_init(void)
    __attribute__((naked)) __attribute__((section(".init1")));

extern "C" void stack_init(void)
{
    __asm volatile(
        "    ldi r30,lo8(_end)\n"
        "    ldi r31,hi8(_end)\n"
        "    ldi r24,lo8(0xc5)\n"
        "    ldi r25,hi8(__stack)\n"
        "    rjmp 2f\n"
        "1:\n"
        "    st Z+,r24\n"
        "2:\n"
        "    cpi r30,lo8(__stack)\n"
        "    cpc r31,r25\n"
        "    brlo 1b\n"
        "    breq 1b" ::);
}

#endif /* ARDUINO_ARCH_AVR */

/* =========================================================================
   Static member definitions
   ========================================================================= */
const uint8_t BACnetHardware::_adcPins[] = {
#ifdef BACNET_ADC_CHANNELS
    PIN_A0, PIN_A1, PIN_A2, PIN_A3,
#endif
};

const uint8_t BACnetHardware::_adcPinCount =
#ifdef BACNET_ADC_CHANNELS
    BACNET_ADC_CHANNELS
#else
    0
#endif
    ;

volatile uint16_t BACnetHardware::_sampleResult[4] = {0, 0, 0, 0};
volatile uint8_t  BACnetHardware::_enabledChannels  = 0;

/* =========================================================================
   GPIO methods
   ========================================================================= */
void BACnetHardware::pinInit(uint8_t pin, bool is_output)
{
#ifdef ARDUINO
#if defined(RGB_BUILTIN)
    if (pin == RGB_BUILTIN)
        return;   /* RGB LED does not need pinMode */
#endif
    pinMode(pin, is_output ? OUTPUT : INPUT);
    if (is_output)
        digitalWrite(pin, LOW);
#else
    (void)pin; (void)is_output;
#endif
}

void BACnetHardware::pinWrite(uint8_t pin, bool active)
{
#ifdef ARDUINO
#if defined(RGB_BUILTIN)
    if (pin == RGB_BUILTIN) {
        rgbLedWrite(RGB_BUILTIN,
                    active ? 64 : 0,
                    active ? 64 : 0,
                    active ? 64 : 0);
        return;
    }
#endif
    digitalWrite(pin, active ? HIGH : LOW);
#else
    (void)pin; (void)active;
#endif
}

bool BACnetHardware::pinRead(uint8_t pin)
{
#ifdef ARDUINO
#if defined(RGB_BUILTIN)
    if (pin == RGB_BUILTIN)
        return false;   /* RGB LED has no readable state */
#endif
    return (digitalRead(pin) == HIGH);
#else
    (void)pin;
    return false;
#endif
}

/* =========================================================================
   ADC methods
   ========================================================================= */
void BACnetHardware::adcInit()
{
    _enabledChannels = 0;

#if defined(ARDUINO_ARCH_ESP32)
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
#elif defined(ARDUINO_ARCH_STM32)
    analogReadResolution(12);
#endif
    /* AVR: default 10-bit, no call needed */

    for (uint8_t i = 0; i < _adcPinCount; i++) {
        _sampleResult[i] = 0;
    }
}

void BACnetHardware::adcEnable(uint8_t index)
{
    if (index >= _adcPinCount)
        return;
    noInterrupts();
    _enabledChannels |= (uint8_t)(1 << index);
    _sampleResult[index] = (uint16_t)analogRead(_adcPins[index]);
    interrupts();
}

uint16_t BACnetHardware::adcResult10bit(uint8_t index)
{
    if (index >= _adcPinCount)
        return 0;
    noInterrupts();
    _sampleResult[index] = (uint16_t)analogRead(_adcPins[index]);
    uint16_t result = _sampleResult[index];
    interrupts();
    return result;
}

uint8_t BACnetHardware::adcResult8bit(uint8_t index)
{
    uint16_t raw = adcResult10bit(index);
#if defined(ARDUINO_ARCH_AVR)
    return (uint8_t)(raw >> 2);   /* 10-bit → 8-bit */
#else
    return (uint8_t)(raw >> 4);   /* 12-bit → 8-bit */
#endif
}

uint16_t BACnetHardware::adcMillivolts(uint8_t index)
{
#if defined(ARDUINO_ARCH_AVR)
    static const float ADC_MAX = 1024.0f;
    static const float MV_MAX  = 5000.0f;
#else
    static const float ADC_MAX = 4095.0f;
    static const float MV_MAX  = 3300.0f;
#endif
    float raw = (float)adcResult10bit(index);
    return (uint16_t)((raw / ADC_MAX) * MV_MAX);
}

uint8_t BACnetHardware::adcChannelCount()
{
    return _adcPinCount;
}

/* =========================================================================
   Stack monitor methods
   ========================================================================= */
void BACnetHardware::stackInit()
{
    /* AVR: real work done by the .init1 free function above — runs before main().
       Non-AVR: no-op (FreeRTOS manages stack). */
}

unsigned BACnetHardware::stackSize()
{
#if defined(ARDUINO_ARCH_AVR) && defined(__GNUC__)
    return (unsigned)((&__stack) - (&_end));
#elif defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_STM32)
#  if defined(STACK_USE_FREERTOS) || defined(ARDUINO_ARCH_ESP32)
    /* FreeRTOS: total stack size not portable, return 0 */
    return 0;
#  else
    return 0;
#  endif
#else
    return 0;
#endif
}

uint8_t BACnetHardware::stackByte(unsigned offset)
{
#if defined(ARDUINO_ARCH_AVR) && defined(__GNUC__)
    return *(&_end + offset);
#else
    (void)offset;
    return 0;
#endif
}

unsigned BACnetHardware::stackUnused()
{
#if defined(ARDUINO_ARCH_AVR) && defined(__GNUC__)
    unsigned count = 0;
    uint8_t *p = &_end;
    while (p <= &__stack) {
        if (*p != STACK_CANARY) {
            count = (unsigned)(p - &_end);
            break;
        }
        p++;
    }
    return count;
#elif defined(ARDUINO_ARCH_ESP32)
    UBaseType_t words = uxTaskGetStackHighWaterMark(NULL);
    return (unsigned)(words * sizeof(StackType_t));
#elif defined(ARDUINO_ARCH_STM32)
#  if __has_include(<FreeRTOS.h>) && __has_include(<task.h>)
    UBaseType_t words = uxTaskGetStackHighWaterMark(NULL);
    return (unsigned)(words * sizeof(StackType_t));
#  else
    return 0;
#  endif
#else
    return 0;
#endif
}

/* =========================================================================
   extern "C" wrappers — same names as the old C functions.
   av.c calls adc_millivolts(), adc_enable(), stack_size(), stack_unused().
   bv.c calls PIN_INIT(), PIN_WRITE().
   These wrappers keep those C files linking without any change.
   ========================================================================= */
extern "C" {

void PIN_INIT(uint8_t pin, bool is_output)  { BACnetHardware::pinInit(pin, is_output); }
void PIN_WRITE(uint8_t pin, bool active)    { BACnetHardware::pinWrite(pin, active); }
bool PIN_READ(uint8_t pin)                  { return BACnetHardware::pinRead(pin); }

void     adc_init(void)                     { BACnetHardware::adcInit(); }
void     adc_enable(uint8_t index)          { BACnetHardware::adcEnable(index); }
uint16_t adc_result_10bit(uint8_t index)    { return BACnetHardware::adcResult10bit(index); }
uint8_t  adc_result_8bit(uint8_t index)     { return BACnetHardware::adcResult8bit(index); }
uint16_t adc_millivolts(uint8_t index)      { return BACnetHardware::adcMillivolts(index); }

/* stack_init on AVR is the .init1 free function above — already extern "C".
   On non-AVR, provide a no-op wrapper so the symbol resolves for C callers. */
#if !defined(ARDUINO_ARCH_AVR)
void     stack_init(void)                   { BACnetHardware::stackInit(); }
#endif
unsigned stack_size(void)                   { return BACnetHardware::stackSize(); }
uint8_t  stack_byte(unsigned offset)        { return BACnetHardware::stackByte(offset); }
unsigned stack_unused(void)                 { return BACnetHardware::stackUnused(); }

} /* extern "C" */
