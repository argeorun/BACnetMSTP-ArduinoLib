/**
 * @file BACnetDevice.h
 * @brief BACnet Device object + APDU/RP/WP/WhoIs handlers — C++ class wrapper
 * @author George Arun
 * @date 2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * Public API for the sketch:
 *   BACnetDevice device;
 *   device.begin();          // call once in setup()
 *   device.update();         // call every loop()
 *   device.AV(92).get();     // read AV instance 92 (returns float)
 *   device.AV(92).set(v);    // write AV instance 92
 *   device.BV(99).get();     // read BV instance 99 (BACNET_BINARY_PV)
 *   device.BV(99).set(v);    // write BV instance 99
 *
 */

#ifndef BACNET_DEVICE_H
#define BACNET_DEVICE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef ARDUINO
#  include <Arduino.h>
#endif

/* BACnet types needed for proxy return types */
#include "../bacnet/bacdef.h"
#include "../bacnet/bacenum.h"
#include "../bacnet/apdu.h"
#include "../bacnet/basic/object/device.h"

/* Forward declarations for proxy types */
class BACnetAV;
class BACnetBV;

/* =========================================================================
   Proxy structs — zero heap, zero overhead, created on the fly
   ========================================================================= */

struct AVProxy {
    uint32_t _instance;
    float get() const;
    bool  set(float value, uint8_t priority = 8) const;
};

struct BVProxy {
    uint32_t _instance;
    BACNET_BINARY_PV get() const;
    bool set(BACNET_BINARY_PV value) const;
    bool set(bool active) const;
};

/* =========================================================================
   BACnetDevice class
   ========================================================================= */

class BACnetDevice {
public:
    void begin();
    void update();

    static AVProxy AV(uint32_t instance) { return AVProxy{instance}; }
    static BVProxy BV(uint32_t instance) { return BVProxy{instance}; }

    /* Exposed for extern "C" wrappers — same signatures as device.c */
    static uint32_t objectInstanceNumber();
    static bool     setObjectInstanceNumber(uint32_t object_id);
    static bool     validObjectInstanceNumber(uint32_t object_id);
    static uint16_t vendorIdentifier();
    static unsigned objectListCount();
    static bool     objectListIdentifier(uint32_t array_index,
                                         BACNET_OBJECT_TYPE *object_type,
                                         uint32_t *instance);
    static int      objectListElementEncode(uint32_t object_instance,
                                            BACNET_ARRAY_INDEX array_index,
                                            uint8_t *apdu);
    static int      readProperty(BACNET_READ_PROPERTY_DATA *rpdata);
    static bool     writeProperty(BACNET_WRITE_PROPERTY_DATA *wp_data);

    /* apdu.c equivalents */
    static bool     apduServiceSupported(BACNET_SERVICES_SUPPORTED svc);
    static void     apduHandler(BACNET_ADDRESS *src, uint8_t *apdu,
                                uint16_t apdu_len);

    /* h_rp.c equivalent */
    static void handlerReadProperty(uint8_t *service_request,
                                    uint16_t service_len,
                                    BACNET_ADDRESS *src,
                                    BACNET_CONFIRMED_SERVICE_DATA *service_data);

    /* h_wp.c equivalent */
    static void handlerWriteProperty(uint8_t *service_request,
                                     uint16_t service_len,
                                     BACNET_ADDRESS *src,
                                     BACNET_CONFIRMED_SERVICE_DATA *service_data);

    /* h_whois.c equivalent */
    static void handlerWhoIs(uint8_t *service_request, uint16_t service_len,
                             BACNET_ADDRESS *src);

private:
    /* Device state — mirrors device.c statics */
    static uint32_t            _objectInstanceNumber;
    static BACNET_DEVICE_STATUS _systemStatus;

    /* h_wp.c: too large for stack on AVR — keep as BSS static */
    static BACNET_WRITE_PROPERTY_DATA _wpData;

    /* Uptime counter */
    static uint32_t _uptimeSeconds;
    static struct mstimer _taskTimer;

    /* Private helpers */
    static int  encodePropertyAPDU(BACNET_READ_PROPERTY_DATA *rpdata);
    static void deviceNvdataInit();
    static void hardwareInit();
    static void oneSecondTask();
};

#endif /* BACNET_DEVICE_H */
