/**
 * @file stack.c
 * @brief C stack usage monitoring — Arduino-compatible implementation
 * @author George Arun (adapted from bacnet-stack AVR port, 2025-2026)
 * @copyright SPDX-License-Identifier: MIT
 * @note Preserves AVR-optimized behavior for ARDUINO_ARCH_AVR/GCC,
 *       with portable fallbacks for other Arduino cores.
 */
#include <stdint.h>
#include "stack.h"

/* AVR/GCC specific symbols and optimized implementation */
#if defined(ARDUINO_ARCH_AVR) && defined(__GNUC__)
extern uint8_t _end;
extern uint8_t __stack;

#define STACK_CANARY (0xC5)
void stack_init(void) __attribute__((naked)) __attribute__((section(".init1")));

void stack_init(void)
{
    /* Inline assembly to initialize stack canary on AVR */
    __asm volatile("    ldi r30,lo8(_end)\n"
                   "    ldi r31,hi8(_end)\n"
                   "    ldi r24,lo8(0xc5)\n" /* STACK_CANARY = 0xc5 */
                   "    ldi r25,hi8(__stack)\n"
                   "    rjmp .cmp\n"
                   ".loop:\n"
                   "    st Z+,r24\n"
                   ".cmp:\n"
                   "    cpi r30,lo8(__stack)\n"
                   "    cpc r31,r25\n"
                   "    brlo .loop\n"
                   "    breq .loop" ::);
}

unsigned stack_size(void)
{
    return (&__stack) - (&_end);
}

uint8_t stack_byte(unsigned offset)
{
    return *(&_end + offset);
}

unsigned stack_unused(void)
{
    unsigned count = 0;
    uint8_t *p = &_end;

    while (p <= &__stack) {
        if ((*p) != STACK_CANARY) {
            count = p - (&_end);
            break;
        }
        p++;
    }

    return count;
}

/* ESP32: use FreeRTOS APIs to get stack high-water mark (bytes) */
#elif defined(ARDUINO_ARCH_ESP32)

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void stack_init(void)
{
    /* No explicit stack canary init required for FreeRTOS tasks */
}

unsigned stack_size(void)
{
    /* Total stack size for the current task is not exposed port-portably; return 0 */
    return 0;
}

uint8_t stack_byte(unsigned offset)
{
    (void)offset;
    return 0;
}

unsigned stack_unused(void)
{
    /* uxTaskGetStackHighWaterMark returns free stack in words */
    UBaseType_t words = uxTaskGetStackHighWaterMark(NULL);
    return (unsigned)(words * sizeof(StackType_t));
}

/* STM32: prefer FreeRTOS APIs when available, otherwise fall back to safe defaults */
#elif defined(ARDUINO_ARCH_STM32)

#if defined(__has_include)
#if __has_include(<FreeRTOS.h>) && __has_include(<task.h>)
#include <FreeRTOS.h>
#include <task.h>
#define STACK_USE_FREERTOS 1
#endif
#endif

#if defined(STACK_USE_FREERTOS)

void stack_init(void)
{
    /* No-op for FreeRTOS */
}

unsigned stack_size(void)
{
    return 0;
}

uint8_t stack_byte(unsigned offset)
{
    (void)offset;
    return 0;
}

unsigned stack_unused(void)
{
    UBaseType_t words = uxTaskGetStackHighWaterMark(NULL);
    return (unsigned)(words * sizeof(StackType_t));
}

#else

void stack_init(void)
{
    /* No-op when neither AVR nor FreeRTOS stack APIs are available */
}

unsigned stack_size(void)
{
    return 0;
}

uint8_t stack_byte(unsigned offset)
{
    (void)offset;
    return 0;
}

unsigned stack_unused(void)
{
    return 0;
}

#endif /* STACK_USE_FREERTOS */

#else

/* Portable fallbacks for other non-AVR Arduino cores */

void stack_init(void)
{
    /* No-op on non-AVR cores; stack canary initialization is platform-specific */
}

unsigned stack_size(void)
{
    /* Unknown at compile time for generic cores; return 0 as safe fallback */
    return 0;
}

uint8_t stack_byte(unsigned offset)
{
    (void)offset;
    return 0;
}

unsigned stack_unused(void)
{
    /* Unknown; return 0 to indicate no measurement available */
    return 0;
}

#endif
