/**
 * @file BACnetNVData.h
 * @brief EEPROM / non-volatile data wrapper for the BACnet library.
 * @author George Arun
 * @date 2025-2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * Wraps src/platform/nvdata.c behind a clean Arduino-style API.
 * The EEPROM backend (Internal AVR / Flash-emulation / External I2C) is
 * selected at compile time via BACNET_EEPROM_BACKEND in compile_config.h.
 * BACnetNVData.cpp provides the eeprom_bytes_read() / eeprom_bytes_write()
 * implementations that nvdata.c calls.
 */

#ifndef BACNET_NVDATA_H
#define BACNET_NVDATA_H

#ifndef __cplusplus
#error "BACnetNVData.h requires a C++ compiler"
#endif

#include "../compile_config.h"  /* must be first — sets BACNET_EEPROM_BACKEND */
#include <stdint.h>
#include <stdbool.h>

extern "C" {
#include "../platform/nvdata.h" /* nvdata_unsigned* functions + NV_EEPROM_* offsets */
}

class BACnetNVData {
public:
    BACnetNVData();

    /**
     * Initialise the storage backend and validate the EEPROM magic marker.
     * For BACNET_EEPROM_FLASH_EMU this calls EEPROM.begin().
     * For BACNET_EEPROM_EXTERNAL_I2C this probes the I²C address; if the
     * chip does not respond within two retries the method returns false and
     * all subsequent writes are silently skipped.
     *
     * @return true if the EEPROM is valid and ready to use; false otherwise.
     */
    bool begin();

    /** @return true if begin() returned true and the magic marker was valid. */
    bool isValid() const;

    /* --- Low-level typed accessors (delegate to nvdata.c) --- */

    uint8_t  readUint8  (uint16_t addr);
    bool     writeUint8 (uint16_t addr, uint8_t  val);

    uint32_t readUint24 (uint16_t addr);
    bool     writeUint24(uint16_t addr, uint32_t val);

    /* --- Non-copyable --- */
    BACnetNVData(const BACnetNVData&)            = delete;
    BACnetNVData& operator=(const BACnetNVData&) = delete;

private:
    bool _valid;
};

#endif /* BACNET_NVDATA_H */
