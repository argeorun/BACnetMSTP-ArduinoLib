/**
 * @file BACnetDevice.h
 * @brief BACnet Device object C++ wrapper.
 * @author George Arun
 * @date 2025-2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * Wraps src/app/device.c via the minimal companion declaration header
 * src/app/device_wrap.h.  Do NOT include src/bacnet/basic/object/device.h
 * here — that upstream header declares Routed_Device_* APIs and
 * BACNET_DEVICE_OBJECT_DESCR which device.c does not implement and would
 * produce link errors.
 *
 * BACnetDevice does NOT hold references to the AV or BV collections — that
 * is the responsibility of BACnetMSTP, the compositor class.
 */

#ifndef BACNET_DEVICE_H
#define BACNET_DEVICE_H

#ifndef __cplusplus
#error "BACnetDevice.h requires a C++ compiler"
#endif

#include "../compile_config.h"  /* must be first */
#include "BACnetObject.h"
#include "BACnetNVData.h"

extern "C" {
#include "../app/device_wrap.h" /* companion declarations for device.c */
}

class BACnetDevice : public BACnetObject {
public:
    BACnetDevice();

    /* ------------------------------------------------------------------ */
    /* EEPROM-persisted identity (R/W over BACnet too)                     */
    /* ------------------------------------------------------------------ */

    /** @return the current device instance number (0–4 194 302). */
    uint32_t    getInstanceNumber() const;            /* KEYWORD2 */
    /**
     * Set the device instance number.  Saves to EEPROM via nvdata.
     * @param id  0–BACNET_MAX_INSTANCE.  Returns false if out of range.
     */
    bool        setInstanceNumber(uint32_t id);       /* KEYWORD2 */

    /** Read the Object_Name from EEPROM. */
    const char* getName() const;                      /* KEYWORD2 */
    /**
     * Write the Object_Name to EEPROM.
     * @param str  1–30 UTF-8 characters.  Returns false if too long.
     */
    bool        setName(const char* str);             /* KEYWORD2 */

    /** Read the Description from EEPROM. */
    const char* getDescription() const;               /* KEYWORD2 */
    /** Write the Description to EEPROM (1–30 chars). */
    bool        setDescription(const char* str);      /* KEYWORD2 */

    /** Read the Location from EEPROM. */
    const char* getLocation() const;                  /* KEYWORD2 */
    /** Write the Location to EEPROM (1–30 chars). */
    bool        setLocation(const char* str);         /* KEYWORD2 */

    /* ------------------------------------------------------------------ */
    /* Pre-compile defaults, overridable at runtime before begin()         */
    /* ------------------------------------------------------------------ */

    /**
     * Model_Name — default: BACNET_DEVICE_MODEL_NAME.
     * Read-only over BACnet; can be overridden at runtime before begin().
     */
    const char* getModelName() const;                 /* KEYWORD2 */
    /** Store a pointer to str (does NOT strcpy — avoids heap on AVR). */
    bool        setModelName(const char* str);        /* KEYWORD2 */

    /**
     * Application_Software_Version — default: BACNET_APP_VERSION.
     * Read-only over BACnet; can be overridden at runtime before begin().
     */
    const char* getAppVersion() const;                /* KEYWORD2 */
    bool        setAppVersion(const char* str);       /* KEYWORD2 */

    /**
     * APDU_Timeout in milliseconds — default: BACNET_APDU_TIMEOUT.
     * Must be called before begin(); has no effect if called after.
     */
    uint32_t    getApduTimeout() const;               /* KEYWORD2 */
    void        setApduTimeout(uint32_t ms);          /* KEYWORD2 */

    /**
     * Number_Of_APDU_Retries — default: BACNET_APDU_RETRIES.
     * Must be called before begin(); has no effect if called after.
     */
    uint8_t     getApduRetries() const;               /* KEYWORD2 */
    void        setApduRetries(uint8_t n);            /* KEYWORD2 */

    /* ------------------------------------------------------------------ */
    /* Pre-compile only — no runtime setter                                */
    /* (Vendor_Name, Vendor_ID, Protocol_Rev, MAX_APDU are compile-time)  */
    /* ------------------------------------------------------------------ */

    /** @return BACNET_VENDOR_NAME from compile_config.h. */
    const char* getVendorName() const;                /* KEYWORD2 */
    /** @return BACNET_VENDOR_ID from compile_config.h. */
    uint16_t    getVendorId() const;                  /* KEYWORD2 */

    /* ------------------------------------------------------------------ */
    /* MS/TP network parameters (EEPROM-persisted)                         */
    /* ------------------------------------------------------------------ */

    uint8_t  getMstpMac() const;                      /* KEYWORD2 */
    /** @param mac  0–127.  Returns false if out of range. */
    bool     setMstpMac(uint8_t mac);                 /* KEYWORD2 */

    uint32_t getMstpBaud() const;                     /* KEYWORD2 */
    /** @param baud  9600 / 19200 / 38400 / 57600 / 76800 / 115200. */
    bool     setMstpBaud(uint32_t baud);              /* KEYWORD2 */

    uint8_t  getMstpMaxMaster() const;                /* KEYWORD2 */
    /** @param max  0–127.  Returns false if out of range. */
    bool     setMstpMaxMaster(uint8_t max);           /* KEYWORD2 */

    /* ------------------------------------------------------------------ */
    /* Lifecycle                                                            */
    /* ------------------------------------------------------------------ */

    /**
     * Initialise the device object.  Called by BACnetMSTP::begin() after
     * BACnetNVData::begin().  Reads EEPROM-persisted values.
     * @param nv  Reference to the already-initialised NVData object.
     */
    void begin(BACnetNVData &nv);

    /* --- BACnetObject contract --- */
    int  readProperty (BACNET_READ_PROPERTY_DATA  *rpdata) override;
    bool writeProperty(BACNET_WRITE_PROPERTY_DATA *wpdata) override;
    bool validInstance(uint32_t id) const override;

    /* --- Non-copyable --- */
    BACnetDevice(const BACnetDevice&)            = delete;
    BACnetDevice& operator=(const BACnetDevice&) = delete;

private:
    const char* _modelName;       /* points to BACNET_DEVICE_MODEL_NAME or user override */
    const char* _appVersion;      /* points to BACNET_APP_VERSION or user override */
    uint32_t    _apduTimeout;
    uint8_t     _apduRetries;
    bool        _initialised;

    /* EEPROM-read name buffers (populated in begin()) */
    char _nameBuf[32];
    char _descBuf[32];
    char _locBuf [32];
};

#endif /* BACNET_DEVICE_H */
