/**
 * @file BACnetStorage.cpp
 * @brief Platform-agnostic EEPROM/flash storage — implementation
 * @author George Arun
 * @date 2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * Platform backend selection:
 *   AVR (Uno, Mega)   — Arduino EEPROM.h  (real on-chip EEPROM)
 *   ESP32 / ESP32-S3  — Arduino EEPROM.h  (flash-sector emulation, needs begin/commit)
 *   STM32             — stm32duino HAL buffer functions directly (see note below)
 *
 * STM32 design note:
 *   stm32duino's EEPROMClass::begin() uses malloc() to allocate its buffer.
 *   On STM32F756ZG, malloc can fail or the flash sector can be left in a
 *   corrupted state after a crash, causing a NULL pointer hard fault on the
 *   next EEPROM.read(). The EEPROMClass is therefore bypassed entirely for STM32.
 *   Instead, the underlying HAL functions are used directly:
 *     eeprom_buffer_fill()          — populate static buffer from flash (no malloc)
 *     eeprom_buffered_read_byte()   — read from static buffer
 *     eeprom_buffered_write_byte()  — write to static buffer
 *   The static buffer (eeprom_emu_buffer[E2END+1] in stm32_eeprom.c) is always
 *   valid regardless of heap state.
 *   eeprom_buffer_flush() (write buffer to flash) is NOT called — it crashes on
 *   F756ZG. STM32 values persist in the static buffer within a session only.
 *   Power-cycle persistence on STM32 is a known limitation (Finding 6).
 */

/* =========================================================================
   Platform EEPROM backend include
   ========================================================================= */
#include <EEPROM.h>
/* STM32: <EEPROM.h> pulls in stm32_eeprom.h which declares the HAL buffer
   functions used below: eeprom_buffer_fill(), eeprom_buffered_read_byte(),
   eeprom_buffered_write_byte(). E2END is also defined there. */

#include "BACnetStorage.h"

/* Pull in the bacint encode/decode helpers used by the typed accessors. */
#include "../bacnet/bacint.h"

/* =========================================================================
   BACnetStorage — lifecycle
   ========================================================================= */

void BACnetStorage::begin()
{
#if defined(ARDUINO_ARCH_ESP32)
    /* ESP32: must call begin(size) before any read or write.
       Without this, all reads return 0 and writes are silently discarded. */
    EEPROM.begin(512);
#elif defined(ARDUINO_ARCH_STM32)
    /* STM32: do NOT call eeprom_buffer_fill(). The EEPROM emulation flash sector
       on F756ZG may be in a corrupted state from a previous crash, causing an
       ECC fault when read. The HAL static buffer (eeprom_emu_buffer[]) starts as
       all zeros from BSS — type tag = 0x0000 ≠ NV_EEPROM_TYPE_ID so the FRESH
       path always runs on boot and writes correct defaults to the buffer.
       No flash read = no ECC fault risk. No power-cycle persistence for STM32
       (known limitation, tracked in Finding 6). */
#endif
    /* AVR: real on-chip EEPROM — no begin() needed. */
}

void BACnetStorage::commit()
{
#if defined(ARDUINO_ARCH_ESP32)
    /* ESP32: flush RAM buffer to flash. Required for power-cycle persistence. */
    EEPROM.commit();
#elif defined(ARDUINO_ARCH_STM32)
    /* STM32: eeprom_buffer_flush() crashes on STM32F756ZG (flash sector erase
       fault — likely ECC error on a sector corrupted by a previous crash).
       Left disabled. Values persist in static buffer within a session only.
       Power-cycle persistence is a known limitation — tracked in Finding 6. */
#endif
    /* AVR: real on-chip EEPROM — writes are immediate, no flush needed. */
}

/* =========================================================================
   extern "C" bridge — implementations
   ========================================================================= */
extern "C" int eeprom_bytes_read(uint16_t eeaddr, uint8_t *buf, int len)
{
    return BACnetStorage::read(eeaddr, buf, len);
}

extern "C" int eeprom_bytes_write(uint16_t eeaddr, const uint8_t *buf, int len)
{
    return BACnetStorage::write(eeaddr, buf, len);
}

/* =========================================================================
   BACnetStorage — raw byte access
   ========================================================================= */
int BACnetStorage::read(uint16_t addr, uint8_t *buf, int len)
{
#if defined(ARDUINO_ARCH_STM32)
    /* Use HAL buffer directly — reads from static eeprom_emu_buffer[].
       Bypasses EEPROMClass to avoid the malloc/NULL-ptr crash path. */
    int count = 0;
    while (len--) {
        buf[count++] = eeprom_buffered_read_byte(addr++);
    }
    return count;
#else
    int count = 0;
    while (len--) {
        buf[count++] = EEPROM.read(addr++);
    }
    return count;
#endif
}

int BACnetStorage::write(uint16_t addr, const uint8_t *buf, int len)
{
#if defined(ARDUINO_ARCH_STM32)
    /* Use HAL buffer directly — writes to static eeprom_emu_buffer[] only.
       No flush to flash (commit() disabled for STM32 — see above). */
    int count = 0;
    while (len-- && addr <= (uint16_t)E2END) {
        eeprom_buffered_write_byte(addr++, buf[count++]);
    }
    return count;
#else
    int count = 0;
    while (len-- && addr < (uint16_t)EEPROM.length()) {
        EEPROM.write(addr++, buf[count++]);
    }
    commit();
    return count;
#endif
}

/* =========================================================================
   BACnetStorage — typed accessors
   Each method uses read()/write() + upstream encode/decode from bacint.c.
   Byte encoding is identical to nvdata.c so existing EEPROM data is
   compatible between base project and new project.
   ========================================================================= */

uint8_t BACnetStorage::unsigned8(uint32_t addr)
{
    uint8_t buf = 0;
    read((uint16_t)addr, &buf, 1);
    return buf;
}

void BACnetStorage::unsigned8_set(uint32_t addr, uint8_t val)
{
    write((uint16_t)addr, &val, 1);
}

uint16_t BACnetStorage::unsigned16(uint32_t addr)
{
    uint16_t val = 0;
    uint8_t buf[2];
    if (read((uint16_t)addr, buf, 2) == 2) {
        decode_unsigned16(buf, &val);
    }
    return val;
}

void BACnetStorage::unsigned16_set(uint32_t addr, uint16_t val)
{
    uint8_t buf[2];
    encode_unsigned16(buf, val);
    write((uint16_t)addr, buf, 2);
}

uint32_t BACnetStorage::unsigned24(uint32_t addr)
{
    uint32_t val = 0;
    uint8_t buf[3];
    if (read((uint16_t)addr, buf, 3) == 3) {
        decode_unsigned24(buf, &val);
    }
    return val;
}

void BACnetStorage::unsigned24_set(uint32_t addr, uint32_t val)
{
    uint8_t buf[3];
    encode_unsigned24(buf, val);
    write((uint16_t)addr, buf, 3);
}

uint32_t BACnetStorage::unsigned32(uint32_t addr)
{
    uint32_t val = 0;
    uint8_t buf[4];
    if (read((uint16_t)addr, buf, 4) == 4) {
        decode_unsigned32(buf, &val);
    }
    return val;
}

void BACnetStorage::unsigned32_set(uint32_t addr, uint32_t val)
{
    uint8_t buf[4];
    encode_unsigned32(buf, val);
    write((uint16_t)addr, buf, 4);
}
