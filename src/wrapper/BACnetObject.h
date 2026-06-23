/**
 * @file BACnetObject.h
 * @brief Abstract base class for all BACnet object wrappers.
 * @author George Arun
 * @date 2025-2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * Header-only — no .cpp counterpart required.
 * Subclasses delegate readProperty() / writeProperty() to
 * the matching C functions in src/app/.
 */

#ifndef BACNET_OBJECT_H
#define BACNET_OBJECT_H

#ifndef __cplusplus
#error "BACnetObject.h requires a C++ compiler"
#endif

#include <stdint.h>
#include <stdbool.h>

/* Pull in BACnet type definitions via extern "C" — avoids name-mangling */
extern "C" {
#include "../bacnet/bacdef.h"   /* BACNET_OBJECT_TYPE, BACNET_ARRAY_INDEX   */
#include "../bacnet/rp.h"       /* BACNET_READ_PROPERTY_DATA                */
#include "../bacnet/wp.h"       /* BACNET_WRITE_PROPERTY_DATA               */
}

class BACnetObject {
public:
    explicit BACnetObject(BACNET_OBJECT_TYPE type) : _type(type) {}
    virtual ~BACnetObject() {}

    /** Returns the BACnet object type constant for this wrapper. */
    BACNET_OBJECT_TYPE objectType() const { return _type; }

    /**
     * Subclasses implement this by delegating to their C Read_Property function.
     * Instance selection is performed inside each C function via
     * rpdata->object_instance — the wrapper never caches an instance number.
     */
    virtual int  readProperty (BACNET_READ_PROPERTY_DATA  *rpdata) = 0;

    /** Subclasses delegate to their C Write_Property function. */
    virtual bool writeProperty(BACNET_WRITE_PROPERTY_DATA *wpdata) = 0;

    /** Range-check: returns true if id is a valid instance number for this type. */
    virtual bool validInstance(uint32_t id) const = 0;

protected:
    BACNET_OBJECT_TYPE _type;
    /*
     * Design note: no _instance field.
     * Storing an instance number here would allow stale values to be used
     * instead of rpdata->object_instance, introducing missed-update bugs.
     * Each C function receives the live instance from rpdata — use that.
     */
};

#endif /* BACNET_OBJECT_H */
