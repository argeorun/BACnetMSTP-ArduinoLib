/**
 * Copyright (c) 2025-2026 George Arun
 *
 * This file is part of the BACnet MSTP Arduino project.
 * Licensed under the MIT License. See LICENSE in the project root.
 */

/**
 * @file compile_config.h
 * @brief Compile-time configuration flags for Arduino BACnet project
 * @note This file replaces compiler flags that would normally be set in Makefile
 * 
 * In the atmega328 project, these are defined in Makefile as:
 *   BFLAGS += -DWRITE_PROPERTY
 *   BFLAGS += -DBACDL_MSTP
 *   etc.
 * 
 * For Arduino, we define them here as this is included before any BACnet headers.
 */

#ifndef COMPILE_CONFIG_H
#define COMPILE_CONFIG_H

/* ============================================================================
   DATALINK CONFIGURATION
   ============================================================================ */

/**
 * @brief Define the BACnet datalink layer to use
 * Options: BACDL_ETHERNET, BACDL_ARCNET, BACDL_MSTP, BACDL_BIP, BACDL_BIP6, etc.
 * Arduino uses MS/TP (Master-Slave/Token-Passing) over RS-485
 */
#define BACDL_MSTP 1

/* ============================================================================
   BACNET SERVICES - Enable/Disable BACnet services
   ============================================================================ */

/**
 * @brief Enable WriteProperty service
 * Allows BACnet clients to write values to object properties
 * Required for remote control of outputs (e.g., Binary Values, Analog Values)
 */
#define WRITE_PROPERTY 1

/**
 * @brief Enable server-side BACnet services
 * This device acts as a BACnet server (responds to client requests)
 */
#define BACNET_SVC_SERVER 1

/* ============================================================================
   BACNET APPLICATION DATA TYPE SUPPORT
   ============================================================================ */

/**
 * @brief Enable REAL (floating point) data type support
 * Required for Analog Value objects with floating point values
 */
#define BACAPP_REAL 1

/**
 * @brief Enable Object Identifier data type support
 * Required for object references and device/object identification
 */
#define BACAPP_OBJECT_ID 1

/**
 * @brief Enable UNSIGNED integer data type support
 * Required for unsigned integer properties
 */
#define BACAPP_UNSIGNED 1

/**
 * @brief Enable ENUMERATED data type support
 * Required for Binary Value (ACTIVE/INACTIVE) and other enumerated properties
 */
#define BACAPP_ENUMERATED 1

/**
 * @brief Enable CHARACTER_STRING data type support
 * Required for object names, descriptions, and text properties
 */
#define BACAPP_CHARACTER_STRING 1

/* ============================================================================
   BACNET PROTOCOL PARAMETERS
   ============================================================================ */

/**
 * @brief BACnet Protocol Revision
 * Revision 9 = BACnet 2004
 * Revision 14 = BACnet 2012
 * Revision 24 = BACnet 2020
 */
#define BACNET_PROTOCOL_REVISION 9

/**
 * @brief Use single-precision float instead of double
 * Set to 0 to use float (4 bytes) instead of double (8 bytes)
 * Saves RAM on resource-constrained devices like Arduino Uno
 */
#define BACNET_USE_DOUBLE 0

/**
 * @brief Maximum APDU size in bytes
 * Arduino Uno has limited RAM (2KB), so we use smaller APDU
 * atmega328: 50 bytes (minimal)
 * Arduino: 128 bytes (still small but more functional)
 * Note: This is also defined in src/bacnet/config.h for Arduino builds
 */
#ifndef MAX_APDU
#define MAX_APDU 128
#endif

/**
 * @brief Maximum number of simultaneous TSM (Transaction State Machine) transactions
 * atmega328: 0 (no client operations)
 * Arduino: 3 (minimal client capability)
 * Set to 0 to disable all client operations (server-only mode)
 */
#ifndef MAX_TSM_TRANSACTIONS
#define MAX_TSM_TRANSACTIONS 3
#endif

/**
 * @brief Byte order for multi-byte values
 * 0 = Little Endian (x86, ARM, AVR)
 * 1 = Big Endian (network byte order, some embedded systems)
 */
#ifndef BIG_ENDIAN
#define BIG_ENDIAN 0
#endif

/* ============================================================================
   BACNET STACK EXPORTS
   ============================================================================ */

/**
 * @brief Define stack export macros
 * Required for proper symbol visibility in BACnet stack
 */
#define BACNET_STACK_EXPORTS 1

/* ============================================================================
   OPTIONAL FEATURES - Currently disabled for Arduino
   ============================================================================ */

/* 
 * Priority Array Support:
 * Full commandable object support with 16-level priority array
 * Disabled for Arduino to save RAM - writes go directly to output
 * Uncomment to enable (requires additional RAM):
 * #define BACNET_PRIORITY_ARRAY 1
 */

/*
 * Intrinsic Reporting (Alarms/Events):
 * Automatic notification when values exceed limits
 * Disabled for Arduino to save flash and RAM
 * #define INTRINSIC_REPORTING 1
 */

/*
 * COV (Change of Value) Reporting:
 * Subscribe to automatic notifications when values change
 * Disabled for Arduino to save resources
 * #define BACNET_COV 1
 */

/*
 * ReadRange Service:
 * Read trend logs and large data arrays
 * Disabled for Arduino (trend logs disabled due to RAM)
 * #define READ_RANGE 1
 */

/* ============================================================================
   DEBUG OUTPUT CONTROL
   Two independent categories. Enabled by default on all boards except UNO
   (UNO has no spare serial port and insufficient RAM for debug output).
   Comment out either define to silence that category.
     BACNET_DEBUG_BOOT    - banner, MAC address, baud rate, stack size at startup
     BACNET_DEBUG_TRAFFIC - [rx] [apdu] [rp] [wp] [whois] [tick] at runtime
   ============================================================================ */
#if !defined(ARDUINO_AVR_UNO)
#  define BACNET_DEBUG_BOOT
// #  define BACNET_DEBUG_TRAFFIC
#endif

/* ============================================================================
   NOTES AND RECOMMENDATIONS
   ============================================================================ */

/*
 * Arduino Uno RAM Constraints:
 * - Total RAM: 2048 bytes
 * - After this configuration: ~2048 bytes used (very tight!)
 * - Main RAM consumer: PDUBuffer[MAX_APDU+16] = ~144 bytes
 * 
 * To save more RAM:
 * 1. Reduce MAX_APDU to 64 or 50 (atmega328 uses 50)
 * 2. Set MAX_TSM_TRANSACTIONS to 0 (server-only, no client)
 * 3. Disable BACAPP_REAL if not using Analog Values
 * 
 * For more features, consider:
 * - Arduino Mega 2560: 8KB RAM (4x more)
 * - ESP32: 520KB RAM (260x more)
 * - STM32: 20-128KB RAM options
 */

#endif /* COMPILE_CONFIG_H */
