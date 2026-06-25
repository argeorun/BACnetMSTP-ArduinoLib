/**
 * @file stm32_rs485_serial_instances.cpp
 * @brief Single-definition STM32 RS-485 HardwareSerial instances.
 * @author George Arun
 * @date 2025-2026
 * @copyright SPDX-License-Identifier: MIT
 */

#include <Arduino.h>

#if defined(ARDUINO_ARCH_STM32)

#if defined(STM32F1xx)
/* STM32F1xx boards (Blue Pill / Black Pill): USART3 pins */
HardwareSerial Serial_RS485(PB11, PB10);
#else
/* Nucleo/F7/H7/L4 boards with GPIOG: USART6 pins */
HardwareSerial Serial6(PG9, PG14);
#endif

#endif
