/**
 * @file BACnetHardware.h
 * @brief Platform pin, ADC, and stack monitoring — replaces pin_config.h, adc.h, stack.h
 * @author George Arun
 * @date 2026
 * @copyright SPDX-License-Identifier: MIT
 *
 */

#ifndef BACNET_HARDWARE_H
#define BACNET_HARDWARE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef ARDUINO
#include <Arduino.h>
#endif

/* =========================================================================
   Board pin macros — identical to base project pin_config.h
   ========================================================================= */
#if defined(ARDUINO_AVR_UNO)

#define PIN_D3   3
#define PIN_D4   4
#define PIN_D5   5
#define PIN_D6   6
#define PIN_D7   7
#define PIN_D8   8
#define PIN_LED  13

#define PIN_I2C_SDA  A4
#define PIN_I2C_SCL  A5
#define PIN_SPI_MOSI 11
#define PIN_SPI_MISO 12
#define PIN_SPI_SCK  13
#define PIN_SPI_SS   10
#define PIN_UART0_TX 1
#define PIN_UART0_RX 0

#elif defined(ARDUINO_AVR_MEGA2560)

#define PIN_D3    3
#define PIN_D4    4
#define PIN_D5    5
#define PIN_D6    6
#define PIN_D7    7
#define PIN_D8    8
#define PIN_LED   13

#define PIN_I2C_SDA  20
#define PIN_I2C_SCL  21
#define PIN_SPI_MOSI 51
#define PIN_SPI_MISO 50
#define PIN_SPI_SCK  52
#define PIN_SPI_SS   53
#define PIN_UART0_TX 1
#define PIN_UART0_RX 0
#define PIN_UART1_TX 18
#define PIN_UART1_RX 19
#define PIN_UART2_TX 16
#define PIN_UART2_RX 17
#define PIN_UART3_TX 14
#define PIN_UART3_RX 15

#elif defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_IDF_TARGET_ESP32S3)

#define PIN_D3       3
#define PIN_D4       14
#define PIN_D5       15
#define PIN_D6       18
#define PIN_D7       47
#define PIN_D8       2

#ifndef RGB_BUILTIN
#  error "RGB_BUILTIN not defined for this ESP32-S3 board"
#endif
#define PIN_LED RGB_BUILTIN

#define PIN_A0       4
#define PIN_A1       5
#define PIN_A2       6
#define PIN_A3       7
#define BACNET_ADC_CHANNELS 4

#define PIN_I2C_SDA  8
#define PIN_I2C_SCL  9
#define PIN_SPI_MOSI 11
#define PIN_SPI_MISO 13
#define PIN_SPI_SCK  12
#define PIN_SPI_SS   10
#define PIN_UART0_TX 43
#define PIN_UART0_RX 44
#define PIN_UART1_TX 21
#define PIN_UART1_RX 22
#define PIN_UART2_TX 17
#define PIN_UART2_RX 16

#elif defined(ARDUINO_ARCH_ESP32)

#define PIN_D3       4
#define PIN_D4       5
#define PIN_D5       19
#define PIN_D6       23
#define PIN_D7       27
#define PIN_D8       18
#define PIN_LED      2

#define PIN_A0       32
#define PIN_A1       33
#define PIN_A2       34
#define PIN_A3       35
#define BACNET_ADC_CHANNELS 4

#define PIN_I2C_SDA  21
#define PIN_I2C_SCL  22
#define PIN_SPI_MOSI 23
#define PIN_SPI_MISO 19
#define PIN_SPI_SCK  18
#define PIN_SPI_SS   5
#define PIN_UART0_TX 1
#define PIN_UART0_RX 3
#define PIN_UART1_TX 25
#define PIN_UART1_RX 26
#define PIN_UART2_TX 17
#define PIN_UART2_RX 16

#elif defined(ARDUINO_ARCH_STM32)

#define PIN_D3       PB3
#define PIN_D4       PB4
#define PIN_D5       PB5
#define PIN_D6       PA10
#define PIN_D7       PA11
#define PIN_D8       PA12
#define PIN_LED      PB7

#define PIN_I2C_SDA  PB8
#define PIN_I2C_SCL  PB9
#define PIN_SPI_MOSI PA7
#define PIN_SPI_MISO PA6
#define PIN_SPI_SCK  PA5
#define PIN_SPI_SS   PA4

#else
#  error "Unsupported board! Please add your board's pin mapping."
#endif

/* =========================================================================
   C API — same function names as pin_config.h, adc.h, stack.h.
   Implemented in BACnetHardware.cpp as extern "C" wrappers around the
   class methods. av.c and bv.c include the old headers (now thin redirects)
   and call these names unchanged.
   ========================================================================= */
#ifdef __cplusplus
extern "C" {
#endif

/* pin_config.h equivalents */
void PIN_INIT(uint8_t pin, bool is_output);
void PIN_WRITE(uint8_t pin, bool active);
bool PIN_READ(uint8_t pin);

/* adc.h equivalents */
void     adc_init(void);
void     adc_enable(uint8_t index);
uint16_t adc_result_10bit(uint8_t index);
uint8_t  adc_result_8bit(uint8_t index);
uint16_t adc_millivolts(uint8_t index);

/* stack.h equivalents */
void     stack_init(void);
unsigned stack_size(void);
uint8_t  stack_byte(unsigned offset);
unsigned stack_unused(void);

#ifdef __cplusplus
}
#endif

/* =========================================================================
   BACnetHardware C++ class — static methods, no instance state
   ========================================================================= */
#ifdef __cplusplus

class BACnetHardware {
public:
    /* --- GPIO ------------------------------------------------------------ */
    static void pinInit(uint8_t pin, bool is_output);
    static void pinWrite(uint8_t pin, bool active);
    static bool pinRead(uint8_t pin);

    /* --- ADC ------------------------------------------------------------- */
    static void     adcInit();
    static void     adcEnable(uint8_t index);
    static uint16_t adcResult10bit(uint8_t index);
    static uint8_t  adcResult8bit(uint8_t index);
    static uint16_t adcMillivolts(uint8_t index);
    static uint8_t  adcChannelCount();

    /* --- Stack monitor --------------------------------------------------- */
    static void     stackInit();   /* AVR: .init1 canary — runs before main() */
    static unsigned stackSize();
    static unsigned stackUnused();
    static uint8_t  stackByte(unsigned offset);

private:
    static const uint8_t        _adcPins[];
    static const uint8_t        _adcPinCount;
    static volatile uint16_t    _sampleResult[4];
    static volatile uint8_t     _enabledChannels;
};

#endif /* __cplusplus */
#endif /* BACNET_HARDWARE_H */
