/**
 * @file stack.h
 * @brief C stack usage monitoring API
 * @author George Arun (adapted from bacnet-stack AVR port, 2025-2026)
 * @copyright SPDX-License-Identifier: MIT
 */
#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* C stack checking */
void stack_init(void);

unsigned stack_size(void);

uint8_t stack_byte(unsigned offset);

unsigned stack_unused(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
