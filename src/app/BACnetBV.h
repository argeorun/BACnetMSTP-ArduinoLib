/**
 * @file BACnetBV.h
 * @brief Binary Value BACnet objects — C++ class wrapper
 * @author George Arun
 * @date 2026
 * @copyright SPDX-License-Identifier: MIT
 *
 */

#ifndef BACNET_BV_H
#define BACNET_BV_H

#include <stdint.h>
#include <stdbool.h>

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "../platform/BACnetHardware.h"
#include "../bacnet/basic/object/bv.h"

class BACnetBV {
public:
    static constexpr unsigned OBJECTS_MAX = 6;

    static void     init();
    static bool     validInstance(uint32_t instance);
    static unsigned count();
    static uint32_t indexToInstance(unsigned idx);
    static unsigned instanceToIndex(uint32_t instance);
    static const char *nameASCII(uint32_t instance);
    static BACNET_BINARY_PV presentValue(uint32_t instance);
    static bool     presentValueSet(uint32_t instance, BACNET_BINARY_PV value);
    static int      readProperty(BACNET_READ_PROPERTY_DATA *rpdata);
    static bool     writeProperty(BACNET_WRITE_PROPERTY_DATA *wp_data);

private:
    struct ObjectData {
        const uint8_t object_id;
        const char   *object_name;
        uint8_t       pin;
        bool          is_output;
    };

    static ObjectData       _objects[OBJECTS_MAX];
    static BACNET_BINARY_PV _presentValue[OBJECTS_MAX];
    static bool             _outOfService[OBJECTS_MAX];

    static ObjectData *objectElement(uint32_t instance);
};

#endif /* BACNET_BV_H */
