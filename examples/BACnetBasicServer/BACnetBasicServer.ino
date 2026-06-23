/**
 * @file BACnetBasicServer.ino
 * @brief BACnet MS/TP server — minimal usage example for the BACnetMSTP library.
 * @author George Arun
 * @date 2025-2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * -----------------------------------------------------------------------
 * QUICK START
 * -----------------------------------------------------------------------
 *  1. Set BACNET_RS485_SERIAL to the HardwareSerial port wired to your
 *     RS-485 transceiver (e.g. Serial1 on Mega/ESP32, Serial on UNO if
 *     you have a second UART).
 *  2. Set BACNET_DERE_PIN to the Arduino pin connected to DE/RE
 *     (active-HIGH driver enable).
 *  3. Upload. The device joins the BACnet MS/TP network.
 *
 * -----------------------------------------------------------------------
 * DEFAULT NETWORK SETTINGS (stored in EEPROM, writable at runtime)
 * -----------------------------------------------------------------------
 *   MS/TP MAC address : 10
 *   Baud rate         : 38400
 *   Max Master        : 127
 *   Device instance   : 260001
 *
 * To change them from a BACnet tool use WriteProperty, or call the
 * bacnet.device() setters before bacnet.begin() (they are applied on
 * next boot from EEPROM).
 *
 * -----------------------------------------------------------------------
 * ANALOG VALUE (AV) OBJECTS  — readable via BACnet ReadProperty
 * -----------------------------------------------------------------------
 *   Instance  Name               Source / Arduino pin   Writable?
 *   --------  -----------------  ---------------------  ---------
 *   AV:0      ADC0               A0 (analog input, mV)  No
 *   AV:1      ADC1               A1 (analog input, mV)  No
 *   AV:2      ADC2               A2 (analog input, mV)  No
 *   AV:3      ADC3               A3 (analog input, mV)  No
 *   AV:92     Device ID          EEPROM                 Yes (0..4194302)
 *   AV:93     MS/TP Baud         EEPROM                 Yes (9600..115200)
 *   AV:94     MS/TP MAC          EEPROM                 Yes (0..127)
 *   AV:95     MS/TP Max Manager  EEPROM                 Yes (0..127)
 *   AV:96     MCU Frequency      CPU clock (Hz)         No
 *   AV:97     CStack Size        C stack total (bytes)  No
 *   AV:98     CStack Unused      C stack headroom       No
 *   AV:99     Uptime             Updated in loop() (h)  No (sketch writes)
 *
 *   To add/change AV objects: edit src/app/av.c → Object_List[]
 *
 * -----------------------------------------------------------------------
 * BINARY VALUE (BV) OBJECTS  — digital outputs (active-HIGH by default)
 * -----------------------------------------------------------------------
 *   Instance  Name   Arduino pin (Uno)   Direction
 *   --------  -----  ------------------  ---------
 *   BV:0      D3     D3                  Output
 *   BV:1      D4     D4                  Output
 *   BV:2      D5     D5                  Output
 *   BV:3      D6     D6                  Output
 *   BV:4      D7     D7                  Output
 *   BV:99     LED    D13 (built-in LED)  Output
 *
 *   To change a BV pin:      edit src/app/bv.c → Object_List[] → pin column
 *   To make a BV an input:   set is_output=false in the same table
 *   Pin name constants:      defined in src/platform/pin_config.h per board
 *
 * -----------------------------------------------------------------------
 * RS-485 / UART PIN ASSIGNMENTS
 * -----------------------------------------------------------------------
 *   Board               UART              TX pin    RX pin    DE/RE pin
 *   ------------------- ----------------  -------   -------   --------------------
 *   Uno / Nano          Serial            D1        D0        BACNET_DERE_PIN (D2)
 *   Mega 2560           Serial1           D18       D19       BACNET_DERE_PIN (D2)
 *   STM32 F103C8        Serial_RS485(*)   PB10      PB11      PA12
 *   STM32 Nucleo-144    Serial6(*)        PG14      PG9       PA12
 *   ESP32               Serial1           GPIO25    GPIO26    GPIO18 (PIN_D8)
 *   ESP32-S3            Serial2           GPIO17    GPIO16    GPIO2  (PIN_D8)
 *
 *   (*) HardwareSerial instance is declared inside the library and
 *       forward-declared (extern) in this sketch — do not rename.
 *
 *   Transceiver wiring:
 *     Arduino TX ──► DI   Arduino RX ◄── RO
 *     DE/RE pin  ──► DE and RE tied together (active-HIGH enables transmit)
 *     A / B      ──► RS-485 bus differential pair
 *
 *   NOTE (Uno): Serial is shared with USB. While the BACnet stack runs,
 *   USB/serial debug output is NOT available on Uno/Nano.
 */

#include <BACnetMSTP.h>

/* ================================================================
 * USER CONFIGURATION — Step 1: Select your board
 * ================================================================
 * Uncomment exactly ONE of the board sections below that matches
 * your hardware. Comment out all the others.
 *
 * The auto-detect block at the bottom works for most IDE users —
 * just select your board in the Arduino IDE and it picks the right
 * settings automatically. Only override manually if auto-detect
 * picks the wrong port or pin.
 * ================================================================ */

/* ----------------------------------------------------------------
 * OPTION A — Arduino Uno / Nano / Pro Mini  (single UART)
 *   RS-485 port : Serial  (TX=D1, RX=D0)
 *   DE/RE pin   : D2
 *   USB debug   : NOT available (Serial is used for RS-485)
 * ---------------------------------------------------------------- */
//#define BACNET_RS485_SERIAL    Serial
//#define BACNET_DERE_PIN        2
//#define BACNET_DEBUG_UART_FREE 0

/* ----------------------------------------------------------------
 * OPTION B — Arduino Mega 2560
 *   RS-485 port : Serial1  (TX1=D18, RX1=D19)
 *   DE/RE pin   : D2
 *   USB debug   : available on Serial
 * ---------------------------------------------------------------- */
//#define BACNET_RS485_SERIAL    Serial1
//#define BACNET_DERE_PIN        2
//#define BACNET_DEBUG_UART_FREE 1

/* ----------------------------------------------------------------
 * OPTION C — STM32 Blue Pill / Black Pill F103C8
 *   RS-485 port : USART3  (TX=PB10, RX=PB11)  — declared in library
 *   DE/RE pin   : PA12
 *   USB debug   : available on Serial (USART1, PA9/PA10)
 *   Note: extern declaration resolves the instance from the library
 * ---------------------------------------------------------------- */
//extern HardwareSerial Serial_RS485;    /* declared in library rs485_port_config.h */
//#define BACNET_RS485_SERIAL    Serial_RS485
//#define BACNET_DERE_PIN        PA12
//#define BACNET_DEBUG_UART_FREE 1

/* ----------------------------------------------------------------
 * OPTION D — STM32 Nucleo-144 (F756ZG / F746ZG / F429ZI etc.)
 *   RS-485 port : USART6  (TX=PG14, RX=PG9)  — declared in library
 *   DE/RE pin   : PA12  (change to any free GPIO on your board)
 *   USB debug   : available on Serial (USART3 / USB-CDC)
 *   Note: extern declaration resolves the instance from the library
 * ---------------------------------------------------------------- */
//extern HardwareSerial Serial6;         /* declared in library rs485_port_config.h */
//#define BACNET_RS485_SERIAL    Serial6
//#define BACNET_DERE_PIN        PA12
//#define BACNET_DEBUG_UART_FREE 1

/* ----------------------------------------------------------------
 * OPTION E — ESP32
 *   RS-485 port : Serial1  (TX=GPIO25, RX=GPIO26)
 *   DE/RE pin   : GPIO2
 *   USB debug   : available on Serial
 * ---------------------------------------------------------------- */
//#define BACNET_RS485_SERIAL    Serial1
//#define BACNET_DERE_PIN        2
//#define BACNET_DEBUG_UART_FREE 1

/* ----------------------------------------------------------------
 * OPTION F — ESP32-S3
 *   RS-485 port : Serial2  (TX=GPIO17, RX=GPIO16)
 *   DE/RE pin   : GPIO2
 *   USB debug   : available on Serial
 * ---------------------------------------------------------------- */
//#define BACNET_RS485_SERIAL    Serial2
//#define BACNET_DERE_PIN        2
//#define BACNET_DEBUG_UART_FREE 1

/* ================================================================
 * AUTO-DETECT (used when none of the above options are uncommented)
 * If you selected your board in the Arduino IDE, this block
 * automatically picks the correct serial port and DE/RE pin.
 * ================================================================ */
#if !defined(BACNET_RS485_SERIAL)

#  if defined(ARDUINO_AVR_UNO)   || defined(ARDUINO_AVR_NANO)  || \
      defined(ARDUINO_AVR_MINI)  || defined(ARDUINO_AVR_PRO)
     /* Uno / Nano — single UART, no debug */
#    define BACNET_RS485_SERIAL    Serial
#    define BACNET_DERE_PIN        2
#    define BACNET_DEBUG_UART_FREE 0

#  elif defined(ARDUINO_ARCH_STM32) && defined(STM32F1xx)
     /* Blue Pill F103C8 — USART3 */
     extern HardwareSerial Serial_RS485;
#    define BACNET_RS485_SERIAL    Serial_RS485
#    define BACNET_DERE_PIN        PA12
#    define BACNET_DEBUG_UART_FREE 1

#  elif defined(ARDUINO_ARCH_STM32)
     /* Nucleo-144 F7/H7/F4 — USART6 */
     extern HardwareSerial Serial6;
#    define BACNET_RS485_SERIAL    Serial6
#    define BACNET_DERE_PIN        PA12
#    define BACNET_DEBUG_UART_FREE 1

#  elif defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_IDF_TARGET_ESP32S3)
     /* ESP32-S3 — Serial2 */
#    define BACNET_RS485_SERIAL    Serial2
#    define BACNET_DERE_PIN        2
#    define BACNET_DEBUG_UART_FREE 1

#  elif defined(ARDUINO_ARCH_ESP32)
     /* ESP32 — Serial1 */
#    define BACNET_RS485_SERIAL    Serial1
#    define BACNET_DERE_PIN        2
#    define BACNET_DEBUG_UART_FREE 1

#  else
     /* Mega 2560 and other multi-UART AVR */
#    define BACNET_RS485_SERIAL    Serial1
#    define BACNET_DERE_PIN        2
#    define BACNET_DEBUG_UART_FREE 1
#  endif

#endif /* !defined(BACNET_RS485_SERIAL) */

/* ============================================================
 * BACnetMSTP instance
 * The serial port and DE/RE pin are mandatory at construction time.
 * ============================================================ */
BACnetMSTP bacnet(BACNET_RS485_SERIAL, BACNET_DERE_PIN);

/* ============================================================
 * Application state
 * ============================================================ */
static uint32_t s_uptime_seconds = 0;
static uint32_t s_last_tick_ms   = 0;

/* AV instance used for uptime reporting */
#define AV_UPTIME_INSTANCE  0U


/* ============================================================
 * setup()
 * ============================================================ */
void setup()
{
#if BACNET_DEBUG_UART_FREE
    /* Optional USB-serial debug output (not available on single-UART boards). */
    Serial.begin(115200);
#endif

    /*
     * Initialise the entire BACnet MS/TP stack in one call.
     * Handles: EEPROM → RS-485 port → Device object → AV/BV arrays →
     *          dlmstp token-passing state machine → millisecond timer → ADC.
     *
     * On first boot (blank EEPROM) factory defaults are written automatically:
     *   MAC=10, baud=38400, maxMaster=127, deviceInstance=260001
     *
     * NOTE — External I2C EEPROM only:
     *   If BACNET_EEPROM_BACKEND == BACNET_EEPROM_EXTERNAL_I2C you must call
     *   Wire.begin() here, BEFORE bacnet.begin().
     */
    if (!bacnet.begin()) {
#if BACNET_DEBUG_UART_FREE
        Serial.println(F("[BACnet] Init failed — check EEPROM / I2C wiring"));
#endif
        /* Halt — no point continuing without a working stack */
        while (true) { delay(1000); }
    }

#if BACNET_DEBUG_UART_FREE
    Serial.print(F("[BACnet] Online  instance="));
    Serial.print(bacnet.device().getInstanceNumber());
    Serial.print(F("  mac="));
    Serial.print(bacnet.device().getMstpMac());
    Serial.print(F("  baud="));
    Serial.println(bacnet.device().getMstpBaud());
#endif

    s_last_tick_ms = millis();
}


/* ============================================================
 * loop()
 * ============================================================ */
void loop()
{
    /*
     * Drive the MS/TP token-passing and APDU receive loop.
     * MUST be called as frequently as possible — do not add long delays here.
     * Internally calls RS485_LED_Timers() + dlmstp_receive() + npdu_handler().
     */
    bacnet.task();

    /* ---- 1-second uptime tick ---------------------------------------- */
    uint32_t now = millis();
    if ((now - s_last_tick_ms) >= 1000UL) {
        s_last_tick_ms += 1000UL;   /* drift-free: add fixed interval, not now */
        s_uptime_seconds++;

        /* Write uptime in hours to AV:0 — visible via BACnet ReadProperty */
        float hours = (float)s_uptime_seconds / 3600.0f;
        bacnet.analogValues().setPresentValue(AV_UPTIME_INSTANCE, hours);
    }
}
