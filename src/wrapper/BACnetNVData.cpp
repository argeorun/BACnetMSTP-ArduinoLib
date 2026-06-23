/**
 * Copyright (c) 2025-2026 George Arun
 *
 * This file is part of the BACnet MSTP Arduino library wrapper layer.
 * Licensed under the MIT License. See LICENSE in the project root.
 */

#include "../compile_config.h"
#include "BACnetNVData.h"

extern "C" {
#include "../platform/nvdata.h"
}

#ifdef ARDUINO
#include <Arduino.h>
#endif

#if BACNET_EEPROM_BACKEND == BACNET_EEPROM_INTERNAL
#include <EEPROM.h>
#elif BACNET_EEPROM_BACKEND == BACNET_EEPROM_FLASH_EMU
#include <EEPROM.h>
#elif BACNET_EEPROM_BACKEND == BACNET_EEPROM_EXTERNAL_I2C
#include <Wire.h>
#ifndef BACNET_EXT_EEPROM_I2C_ADDR
#define BACNET_EXT_EEPROM_I2C_ADDR 0x50
#endif
#ifndef BACNET_EXT_EEPROM_PAGE_SIZE
#define BACNET_EXT_EEPROM_PAGE_SIZE 32
#endif
#else
#error "Unsupported BACNET_EEPROM_BACKEND"
#endif

extern "C" {

/* C linkage EEPROM bridge used by nvdata.c */
int eeprom_bytes_read(uint16_t eeaddr, uint8_t *buf, int len) {
#if BACNET_EEPROM_BACKEND == BACNET_EEPROM_INTERNAL
    int count = 0;
    while (len--) buf[count++] = EEPROM.read(eeaddr++);
    return count;
#elif BACNET_EEPROM_BACKEND == BACNET_EEPROM_FLASH_EMU
    int count = 0;
    while (len--) buf[count++] = EEPROM.read(eeaddr++);
    return count;
#elif BACNET_EEPROM_BACKEND == BACNET_EEPROM_EXTERNAL_I2C
    int count = 0;
    while (len--) {
        Wire.beginTransmission(BACNET_EXT_EEPROM_I2C_ADDR);
        Wire.write((uint8_t)(eeaddr >> 8));
        Wire.write((uint8_t)(eeaddr & 0xFF));
        if (Wire.endTransmission(false) != 0) return count; // NACK
        Wire.requestFrom((int)BACNET_EXT_EEPROM_I2C_ADDR, 1);
        if (Wire.available()) buf[count++] = Wire.read();
        eeaddr++;
    }
    return count;
#else
    return 0;
#endif
}

int eeprom_bytes_write(uint16_t eeaddr, const uint8_t *buf, int len) {
#if BACNET_EEPROM_BACKEND == BACNET_EEPROM_INTERNAL
    int count = 0;
    while (len-- && eeaddr < (uint16_t)EEPROM.length()) EEPROM.write(eeaddr++, buf[count++]);
    return count;
#elif BACNET_EEPROM_BACKEND == BACNET_EEPROM_FLASH_EMU
    int count = 0;
#if defined(ARDUINO_ARCH_ESP32)
    // ESP32 EEPROM emulation needs begin(size) called elsewhere; write then commit
    while (len-- && eeaddr < (uint16_t)EEPROM.length()) EEPROM.write(eeaddr++, buf[count++]);
    EEPROM.commit();
    return count;
#else
    while (len-- && eeaddr < (uint16_t)EEPROM.length()) EEPROM.write(eeaddr++, buf[count++]);
    return count;
#endif
#elif BACNET_EEPROM_BACKEND == BACNET_EEPROM_EXTERNAL_I2C
    int count = 0;
    while (len > 0) {
        uint16_t page_offset = eeaddr % BACNET_EXT_EEPROM_PAGE_SIZE;
        uint16_t space = BACNET_EXT_EEPROM_PAGE_SIZE - page_offset;
        uint16_t to_write = (len < space) ? len : space;
        Wire.beginTransmission(BACNET_EXT_EEPROM_I2C_ADDR);
        Wire.write((uint8_t)(eeaddr >> 8));
        Wire.write((uint8_t)(eeaddr & 0xFF));
        for (uint16_t i = 0; i < to_write; i++) Wire.write(buf[count+i]);
        if (Wire.endTransmission() != 0) return count; // NACK
        delay(5); // page-write delay
        eeaddr += to_write;
        count += to_write;
        len -= to_write;
    }
    return count;
#else
    return 0;
#endif
}

} // extern "C"

/* -------------------------------------------------------------------------
 * BACnetNVData C++ wrapper implementation
 * ------------------------------------------------------------------------- */

BACnetNVData::BACnetNVData(): _valid(false) {}

bool BACnetNVData::begin() {
#if BACNET_EEPROM_BACKEND == BACNET_EEPROM_FLASH_EMU
#if defined(ARDUINO_ARCH_ESP32)
    // Ensure EEPROM emulation initialized; safe to call multiple times
    EEPROM.begin(512);
#endif
#endif
    uint16_t type_id = nvdata_unsigned16(NV_EEPROM_TYPE_0);
    uint8_t  version = nvdata_unsigned8(NV_EEPROM_VERSION);
    _valid = (type_id == NV_EEPROM_TYPE_ID) && (version == NV_EEPROM_VERSION_ID);
    return _valid;
}

bool BACnetNVData::isValid() const { return _valid; }

uint8_t BACnetNVData::readUint8(uint16_t addr) { return nvdata_unsigned8(addr); }
bool BACnetNVData::writeUint8(uint16_t addr, uint8_t val) { return nvdata_unsigned8_set(addr, val) >= 0; }
uint32_t BACnetNVData::readUint24(uint16_t addr) { return nvdata_unsigned24(addr); }
bool BACnetNVData::writeUint24(uint16_t addr, uint32_t val) { return nvdata_unsigned24_set(addr, val) >= 0; }
