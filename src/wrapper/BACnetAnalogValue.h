/**
 * @file BACnetAnalogValue.h
 * @brief BACnet Analog Value object C++ wrapper.
 * @author George Arun
 * @date 2025-2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * Wraps src/app/av.c via the minimal companion declaration header
 * src/app/av_wrap.h.  Do NOT include src/bacnet/basic/object/av.h here —
 * that upstream header declares symbols av.c does not implement and would
 * produce link errors.
 */

#ifndef BACNET_ANALOG_VALUE_H
#define BACNET_ANALOG_VALUE_H

#ifndef __cplusplus
#error "BACnetAnalogValue.h requires a C++ compiler"
#endif

#include "../compile_config.h"  /* must be first */
#include "BACnetObject.h"

extern "C" {
#include "../app/av_wrap.h"     /* companion declarations for av.c */
}

class BACnetAnalogValue : public BACnetObject {
public:
    BACnetAnalogValue();

    /* --- Arduino-style property accessors --- */

    /** Read the Present_Value for the given instance. */
    float    getPresentValue(uint32_t instance);              /* KEYWORD2 */
    /**
     * Write the Present_Value for the given instance.
     * @param priority  BACnet priority array level (1–16); 0 uses default.
     */
    bool     setPresentValue(uint32_t instance, float value, uint8_t priority = 0); /* KEYWORD2 */

    /** Read the Engineering_Units property (returns BACNET_ENGINEERING_UNITS cast to uint16_t). */
    uint16_t getUnits       (uint32_t instance);              /* KEYWORD2 */
    /** Write the Engineering_Units property. */
    bool     setUnits       (uint32_t instance, uint16_t units); /* KEYWORD2 */

    /** @return true if the instance accepts BACnet WriteProperty for Present_Value. */
    bool     isWritable     (uint32_t instance);              /* KEYWORD2 */

    /** @return the total number of Analog Value instances. */
    unsigned count          () const;                         /* KEYWORD2 */

    /* --- BACnetObject contract --- */
    int  readProperty (BACNET_READ_PROPERTY_DATA  *rpdata) override;
    bool writeProperty(BACNET_WRITE_PROPERTY_DATA *wpdata) override;
    bool validInstance(uint32_t id) const override;
};

#endif /* BACNET_ANALOG_VALUE_H */
