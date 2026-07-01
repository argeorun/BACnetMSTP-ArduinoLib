/**
 * @file BACnetStorage.h
 * @brief Platform-agnostic EEPROM/flash storage class
 * @author George Arun
 * @date 2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * Replaces: nvdata EEPROM bridge in BACnet-MSTP-Arduino.ino
 *
 * Provides:
 *   - Static read/write methods wrapping the platform EEPROM backend
 *     (AVR/ESP32: EEPROM.h  |  STM32: FlashStorage_STM32.h)
 *   - extern "C" bridge declarations for eeprom_bytes_read / eeprom_bytes_write
 *     (required by nvdata.c which is pure C and calls these by name)
 *   - Typed accessor methods matching the nvdata.c API so C++ callers
 *     (the .ino and BACnetDevice) do not need to include nvdata.h directly
 *
 * nvdata.c is retained unchanged. It calls eeprom_bytes_read/write via the
 * C bridge implemented in BACnetStorage.cpp.
 */

#ifndef BACNET_STORAGE_H
#define BACNET_STORAGE_H

#include <stdint.h>
#include <stdbool.h>

/* =========================================================================
   C linkage bridge — MUST be declared here so every translation unit that
   includes this header sees the extern "C" prototype.
   nvdata.c resolves these symbols at link time; the definitions live in
   BACnetStorage.cpp.
   ========================================================================= */
#ifdef __cplusplus
extern "C" {
#endif

int eeprom_bytes_read(uint16_t eeaddr, uint8_t *buf, int len);
int eeprom_bytes_write(uint16_t eeaddr, const uint8_t *buf, int len);

#ifdef __cplusplus
}
#endif

/* =========================================================================
   BACnetStorage class — all methods static, no instance state
   ========================================================================= */
#ifdef __cplusplus

class BACnetStorage {
public:
    /* --- Lifecycle ------------------------------------------------------- */

    /* ESP32 requires EEPROM.begin(size) before any read/write.
       Call once from hardware_init() before any nvdata calls.
       No-op on AVR and STM32. */
    static void begin();

    /* Flush writes to flash. Called automatically at end of write().
       ESP32: EEPROM.commit()  |  STM32: eeprom_buffer_flush()  |  AVR: no-op */
    static void commit();

    /* --- Raw byte access ------------------------------------------------- */

    /* Read len bytes from EEPROM starting at addr into buf.
       @return number of bytes read */
    static int read(uint16_t addr, uint8_t *buf, int len);

    /* Write len bytes from buf to EEPROM starting at addr.
       Bounds-checked against EEPROM.length().
       Calls commit() automatically on ESP32.
       @return number of bytes written */
    static int write(uint16_t addr, const uint8_t *buf, int len);

    /* --- Typed accessors — same semantics as nvdata.c functions ---------- */

    static uint8_t  unsigned8(uint32_t addr);
    static void     unsigned8_set(uint32_t addr, uint8_t val);

    static uint16_t unsigned16(uint32_t addr);
    static void     unsigned16_set(uint32_t addr, uint16_t val);

    static uint32_t unsigned24(uint32_t addr);
    static void     unsigned24_set(uint32_t addr, uint32_t val);

    static uint32_t unsigned32(uint32_t addr);
    static void     unsigned32_set(uint32_t addr, uint32_t val);
};

#endif /* __cplusplus */
#endif /* BACNET_STORAGE_H */
