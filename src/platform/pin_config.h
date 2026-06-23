/*
 * Copyright (c) 2025-2026 George Arun
 *
 * This file is part of the BACnet MSTP Arduino project.
 * Licensed under the MIT License. See LICENSE in the project root.
 */

#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H


#include <stdint.h>
#include <stdbool.h>

#ifdef ARDUINO
#include <Arduino.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif
/* ============================================================
   AUTO BOARD DETECTION
   ============================================================ */
#if defined(ARDUINO_AVR_UNO)
/************** ARDUINO UNO **************/


#define PIN_D3   3
#define PIN_D4   4
#define PIN_D5   5
#define PIN_D6   6
#define PIN_D7   7
#define PIN_D8   8


#define PIN_LED  13

/* ANALOG pins are default pins so need to define */
/*
#define PIN_A0   A0
#define PIN_A1   A1
#define PIN_A2   A2
#define PIN_A3   A3
#define PIN_A4   A4
#define PIN_A5   A5 
*/

/* I2C */
#define PIN_I2C_SDA  A4
#define PIN_I2C_SCL  A5

/* SPI */
#define PIN_SPI_MOSI 11
#define PIN_SPI_MISO 12
#define PIN_SPI_SCK  13
#define PIN_SPI_SS   10

/* UART */
#define PIN_UART0_TX 1
#define PIN_UART0_RX 0


#elif defined(ARDUINO_AVR_MEGA2560)
/************** ARDUINO MEGA 2560 **************/

#define PIN_D3    3
#define PIN_D4    4
#define PIN_D5    5
#define PIN_D6    6
#define PIN_D7    7
#define PIN_D8    8

#define PIN_LED   13

/* ANALOG  pins are default pins so no need to define */
/*
#define PIN_A0    A0
#define PIN_A1    A1
#define PIN_A2    A2
#define PIN_A3    A3
#define PIN_A4    A4
#define PIN_A5    A5
*/

/* I2C */
#define PIN_I2C_SDA  20
#define PIN_I2C_SCL  21

/* SPI */
#define PIN_SPI_MOSI 51
#define PIN_SPI_MISO 50
#define PIN_SPI_SCK  52
#define PIN_SPI_SS   53


/* UART */
#define PIN_UART0_TX 1
#define PIN_UART0_RX 0

#define PIN_UART1_TX 18
#define PIN_UART1_RX 19

#define PIN_UART2_TX 16
#define PIN_UART2_RX 17

#define PIN_UART3_TX 14
#define PIN_UART3_RX 15

#elif defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_IDF_TARGET_ESP32S3)
/************** ESP32-S3 **************/

#define PIN_D3       3
#define PIN_D4       14
#define PIN_D5       15
#define PIN_D6       18
#define PIN_D7       47
#define PIN_D8       2
 

#ifndef RGB_BUILTIN
  #error "RGB_BUILTIN not defined for this ESP32-S3 board"
#endif

#define PIN_LED RGB_BUILTIN

/* ANALOG */
#define PIN_A0       4
#define PIN_A1       5
#define PIN_A2       6
#define PIN_A3       7

/* I2C */
#define PIN_I2C_SDA  8
#define PIN_I2C_SCL  9

/* SPI */
#define PIN_SPI_MOSI 11
#define PIN_SPI_MISO 13
#define PIN_SPI_SCK  12
#define PIN_SPI_SS   10   // SS only (no LED conflict)


/* UART */
#define PIN_UART0_TX 43
#define PIN_UART0_RX 44

#define PIN_UART1_TX 21
#define PIN_UART1_RX 22

#define PIN_UART2_TX 17
#define PIN_UART2_RX 16


#elif defined(ARDUINO_ARCH_ESP32)
/************** ESP32 **************/

#define PIN_D3       4
#define PIN_D4       5
#define PIN_D5       19
#define PIN_D6       23
#define PIN_D7       27
#define PIN_D8       18

/* LED */
#define PIN_LED      2
  

/* ANALOG */
#define PIN_A0       32
#define PIN_A1       33
#define PIN_A2       34   
#define PIN_A3       35   

/* I2C */
#define PIN_I2C_SDA  21
#define PIN_I2C_SCL  22

/* SPI (VSPI) */
#define PIN_SPI_MOSI 23
#define PIN_SPI_MISO 19
#define PIN_SPI_SCK  18
#define PIN_SPI_SS   5

/* UART */
#define PIN_UART0_TX 1
#define PIN_UART0_RX 3

#define PIN_UART1_TX 25
#define PIN_UART1_RX 26

#define PIN_UART2_TX 17
#define PIN_UART2_RX 16

#elif defined(ARDUINO_ARCH_STM32)
/************** STM32 **************/


#define PIN_D3       PB3
#define PIN_D4       PB4
#define PIN_D5       PB5
#define PIN_D6       PA10
#define PIN_D7       PA11
#define PIN_D8       PA12

/* LED */
#define PIN_LED      PB7  //BLUE_LED

/* ANALOG pins are default pins so need to define */
//#define PIN_A0       PA3
//#define PIN_A1       PC0
//#define PIN_A2       PC3
//#define PIN_A3       PF3

/* I2C */
#define PIN_I2C_SDA  PB8
#define PIN_I2C_SCL  PB9

/* SPI */
#define PIN_SPI_MOSI PA7
#define PIN_SPI_MISO PA6
#define PIN_SPI_SCK  PA5
#define PIN_SPI_SS   PA4

/* UART */
//#define PIN_UART1_TX PA9
//#define PIN_UART1_RX PA10


#else
#error "Unsupported board! Please add your board's pin mapping."
#endif

/* ============================================================
   FUNCTIONS
   ============================================================ */
void PIN_INIT(uint8_t pin, bool is_output);
void PIN_WRITE(uint8_t pin, bool active);
bool PIN_READ(uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif

