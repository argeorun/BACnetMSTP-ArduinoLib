/**
 * @file BACnetDevice.cpp
 * @brief BACnet Device object C++ wrapper — implementation.
 * @author George Arun
 * @date 2025-2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * Delegates reads/writes over BACnet to device.c functions exposed via
 * device_wrap.h.  EEPROM-persisted properties (instance number, name,
 * description, location, MS/TP settings) are read directly from nvdata.c
 * because device.c handles them internally via its Read_Property handler.
 */

#if !defined(ARDUINO)
#error "BACnetDevice.cpp is Arduino-only; do not include it in desktop builds."
#endif

#include "BACnetDevice.h"
#include <string.h>     /* strlen, strncpy */

extern "C" {
#include "../platform/nvdata.h"   /* nvdata_name, nvdata_unsigned24, etc. */
#include "../platform/rs485.h"    /* RS485_Baud_Rate_From_Kilo             */
#include "../bacnet/bacstr.h"     /* BACNET_CHARACTER_STRING              */
}

BACnetDevice::BACnetDevice()
    : BACnetObject(OBJECT_DEVICE)
    , _modelName(BACNET_DEVICE_MODEL_NAME)
    , _appVersion(BACNET_APP_VERSION)
    , _apduTimeout(BACNET_APDU_TIMEOUT)
    , _apduRetries(BACNET_APDU_RETRIES)
    , _initialised(false)
{
    _nameBuf[0] = '\0';
    _descBuf[0] = '\0';
    _locBuf [0] = '\0';
}

void BACnetDevice::begin(BACnetNVData &nv)
{
    (void)nv; /* nv already validated by BACnetMSTP::begin() */

    /* Load EEPROM-persisted instance number */
    uint32_t id = nvdata_unsigned24(NV_EEPROM_DEVICE_0);
    if (id <= BACNET_MAX_INSTANCE) {
        Device_Set_Object_Instance_Number(id);
    }

    /* Cache name strings from EEPROM into local char buffers.
     * nvdata_name() writes into a BACNET_CHARACTER_STRING; we then copy
     * the C string into our fixed buffer.                                */
    BACNET_CHARACTER_STRING cs;
    nvdata_name(NV_EEPROM_DEVICE_NAME,        &cs, "BACnet Device");
    strncpy(_nameBuf, characterstring_value(&cs), sizeof(_nameBuf) - 1);
    _nameBuf[sizeof(_nameBuf) - 1] = '\0';

    nvdata_name(NV_EEPROM_DEVICE_DESCRIPTION, &cs, "Description");
    strncpy(_descBuf, characterstring_value(&cs), sizeof(_descBuf) - 1);
    _descBuf[sizeof(_descBuf) - 1] = '\0';

    nvdata_name(NV_EEPROM_DEVICE_LOCATION,    &cs, "Location");
    strncpy(_locBuf,  characterstring_value(&cs), sizeof(_locBuf)  - 1);
    _locBuf[sizeof(_locBuf) - 1] = '\0';

    _initialised = true;
}

/* =========================================================================
 * EEPROM-persisted identity
 * ========================================================================= */

uint32_t BACnetDevice::getInstanceNumber() const
{
    return Device_Object_Instance_Number();
}

bool BACnetDevice::setInstanceNumber(uint32_t id)
{
    if (id > BACNET_MAX_INSTANCE) {
        return false;
    }
    bool ok = Device_Set_Object_Instance_Number(id);
    if (ok) {
        nvdata_unsigned24_set(NV_EEPROM_DEVICE_0, id);
    }
    return ok;
}

const char* BACnetDevice::getName() const
{
    return _nameBuf;
}

bool BACnetDevice::setName(const char* str)
{
    if (!str || strlen(str) == 0 || strlen(str) > NV_EEPROM_NAME_SIZE) {
        return false;
    }
    bool ok = nvdata_name_set(NV_EEPROM_DEVICE_NAME,
                              CHARACTER_UTF8,
                              str, (uint8_t)strlen(str));
    if (ok) {
        strncpy(_nameBuf, str, sizeof(_nameBuf) - 1);
        _nameBuf[sizeof(_nameBuf) - 1] = '\0';
    }
    return ok;
}

const char* BACnetDevice::getDescription() const
{
    return _descBuf;
}

bool BACnetDevice::setDescription(const char* str)
{
    if (!str || strlen(str) == 0 || strlen(str) > NV_EEPROM_NAME_SIZE) {
        return false;
    }
    bool ok = nvdata_name_set(NV_EEPROM_DEVICE_DESCRIPTION,
                              CHARACTER_UTF8,
                              str, (uint8_t)strlen(str));
    if (ok) {
        strncpy(_descBuf, str, sizeof(_descBuf) - 1);
        _descBuf[sizeof(_descBuf) - 1] = '\0';
    }
    return ok;
}

const char* BACnetDevice::getLocation() const
{
    return _locBuf;
}

bool BACnetDevice::setLocation(const char* str)
{
    if (!str || strlen(str) == 0 || strlen(str) > NV_EEPROM_NAME_SIZE) {
        return false;
    }
    bool ok = nvdata_name_set(NV_EEPROM_DEVICE_LOCATION,
                              CHARACTER_UTF8,
                              str, (uint8_t)strlen(str));
    if (ok) {
        strncpy(_locBuf, str, sizeof(_locBuf) - 1);
        _locBuf[sizeof(_locBuf) - 1] = '\0';
    }
    return ok;
}

/* =========================================================================
 * Pre-compile defaults (overridable at runtime before begin())
 * ========================================================================= */

const char* BACnetDevice::getModelName() const
{
    return _modelName;
}

bool BACnetDevice::setModelName(const char* str)
{
    if (!str || strlen(str) == 0) {
        return false;
    }
    _modelName = str;   /* store pointer — no strcpy to avoid heap on AVR */
    return true;
}

const char* BACnetDevice::getAppVersion() const
{
    return _appVersion;
}

bool BACnetDevice::setAppVersion(const char* str)
{
    if (!str || strlen(str) == 0) {
        return false;
    }
    _appVersion = str;
    return true;
}

uint32_t BACnetDevice::getApduTimeout() const
{
    return _apduTimeout;
}

void BACnetDevice::setApduTimeout(uint32_t ms)
{
#ifdef DEBUG
    if (_initialised) {
        Serial.println(F("BACnetDevice: setApduTimeout() called after begin() — no effect"));
    }
#endif
    if (!_initialised) {
        _apduTimeout = ms;
    }
}

uint8_t BACnetDevice::getApduRetries() const
{
    return _apduRetries;
}

void BACnetDevice::setApduRetries(uint8_t n)
{
#ifdef DEBUG
    if (_initialised) {
        Serial.println(F("BACnetDevice: setApduRetries() called after begin() — no effect"));
    }
#endif
    if (!_initialised) {
        _apduRetries = n;
    }
}

/* =========================================================================
 * Pre-compile only (vendor identity)
 * ========================================================================= */

const char* BACnetDevice::getVendorName() const
{
    return BACNET_VENDOR_NAME;
}

uint16_t BACnetDevice::getVendorId() const
{
    return (uint16_t)BACNET_VENDOR_ID;
}

/* =========================================================================
 * MS/TP network parameters
 * ========================================================================= */

uint8_t BACnetDevice::getMstpMac() const
{
    return nvdata_unsigned8(NV_EEPROM_MSTP_MAC);
}

bool BACnetDevice::setMstpMac(uint8_t mac)
{
    if (mac > 127) {
        return false;
    }
    nvdata_unsigned8_set(NV_EEPROM_MSTP_MAC, mac);
    return true;
}

uint32_t BACnetDevice::getMstpBaud() const
{
    uint8_t baud_k = nvdata_unsigned8(NV_EEPROM_MSTP_BAUD_K);
    return RS485_Baud_Rate_From_Kilo(baud_k);
}

bool BACnetDevice::setMstpBaud(uint32_t baud)
{
    /* Valid rates from RS485_Set_Baud_Rate() */
    uint8_t baud_k;
    switch (baud) {
        case 9600:   baud_k = 9;   break;
        case 19200:  baud_k = 19;  break;
        case 38400:  baud_k = 38;  break;
        case 57600:  baud_k = 57;  break;
        case 76800:  baud_k = 76;  break;
        case 115200: baud_k = 115; break;
        default: return false;
    }
    nvdata_unsigned8_set(NV_EEPROM_MSTP_BAUD_K, baud_k);
    return true;
}

uint8_t BACnetDevice::getMstpMaxMaster() const
{
    return nvdata_unsigned8(NV_EEPROM_MSTP_MAX_MASTER);
}

bool BACnetDevice::setMstpMaxMaster(uint8_t max)
{
    if (max > 127) {
        return false;
    }
    nvdata_unsigned8_set(NV_EEPROM_MSTP_MAX_MASTER, max);
    return true;
}

/* =========================================================================
 * BACnetObject contract
 * ========================================================================= */

int BACnetDevice::readProperty(BACNET_READ_PROPERTY_DATA *rpdata)
{
    return Device_Read_Property(rpdata);
}

bool BACnetDevice::writeProperty(BACNET_WRITE_PROPERTY_DATA *wpdata)
{
    return Device_Write_Property(wpdata);
}

bool BACnetDevice::validInstance(uint32_t id) const
{
    return Device_Valid_Object_Instance_Number(id);
}
