/**
 * @file rs485_port_config.h
 * @brief RS-485 HardwareSerial port selection
 * @author George Arun
 * @date 2025-2026
 * @copyright SPDX-License-Identifier: MIT
 */
// Optional: define RS485_PORT here to select the HardwareSerial used for RS485.
// Default for Arduino Mega 2560: use Serial1 (TX1=18, RX1=19).
#ifndef RS485_PORT

#if defined(ARDUINO_ARCH_STM32)
  #if defined(STM32F1xx)
    /* STM32F1 (Blue Pill F103C8/CB, Black Pill F103, etc.)
     * USART3: PB10 (TX), PB11 (RX) — declare instance since it's not pre-instantiated.
     * HardwareSerial(RX_pin, TX_pin) constructor convention in STM32duino. */
    HardwareSerial Serial_RS485(PB11, PB10);
    #define RS485_PORT Serial_RS485
  #else
    /* Other STM32 (Nucleo-F7/H7/L4, etc.) with GPIOG available
     * Serial6 = USART6: PG14 (TX), PG9 (RX) */
    HardwareSerial Serial6(PG9, PG14);
    #define RS485_PORT Serial6
  #endif

#elif defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_IDF_TARGET_ESP32S3)
  #define RS485_PORT Serial2

#else
  #define RS485_PORT Serial

#endif
#endif

/* Note: RS485_PORT can be overridden by defining it before including this header,
 * or by calling RS485_Set_Serial_Port() at runtime in your sketch. */
