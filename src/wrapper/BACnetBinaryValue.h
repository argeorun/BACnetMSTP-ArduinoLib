/**
 * @file BACnetBinaryValue.h
 * @brief BACnet Binary Value object C++ wrapper.
 * @author George Arun
 * @date 2025-2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * Wraps src/app/bv.c via the minimal companion declaration header
 * src/app/bv_wrap.h.  Do NOT include src/bacnet/basic/object/bv.h here —
 * that upstream header declares symbols bv.c does not implement and would
 * produce link errors.
 */

#ifndef BACNET_BINARY_VALUE_H
#define BACNET_BINARY_VALUE_H

#ifndef __cplusplus
#error "BACnetBinaryValue.h requires a C++ compiler"
#endif

#include "../compile_config.h"  /* must be first */
#include "BACnetObject.h"

extern "C" {
#include "../app/bv_wrap.h"     /* companion declarations for bv.c */
}

class BACnetBinaryValue : public BACnetObject {
public:
    /**
     * Typed state enum — maps BACNET_BINARY_PV directly.
     * INACTIVE == BINARY_INACTIVE (0), ACTIVE == BINARY_ACTIVE (1).
     * Compile-time assertion in BACnetBinaryValue.cpp validates the mapping.
     */
    enum State { INACTIVE = 0, ACTIVE = 1 };   /* KEYWORD1 values */

    BACnetBinaryValue();

    /* --- Arduino-style property accessors --- */

    /** Read the Present_Value for the given instance. */
    State   getState (uint32_t instance);                    /* KEYWORD2 */
    /**
     * Write the Present_Value for the given instance.
     * Returns false if the instance does not accept writes (isOutput() == false).
     */
    bool    setState (uint32_t instance, State s);           /* KEYWORD2 */

    /** @return the Arduino digital pin number mapped to this instance. */
    uint8_t getPin   (uint32_t instance);                    /* KEYWORD2 */

    /** @return true if the instance is an output (write requests accepted). */
    bool    isOutput (uint32_t instance);                    /* KEYWORD2 */

    /** @return the total number of Binary Value instances. */
    unsigned count   () const;                               /* KEYWORD2 */

    /* --- BACnetObject contract --- */
    int  readProperty (BACNET_READ_PROPERTY_DATA  *rpdata) override;
    bool writeProperty(BACNET_WRITE_PROPERTY_DATA *wpdata) override;
    bool validInstance(uint32_t id) const override;
};

#endif /* BACNET_BINARY_VALUE_H */
