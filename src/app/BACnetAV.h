/**
 * @file BACnetAV.h
 * @brief Analog Value BACnet objects — C++ class wrapper
 * @author George Arun
 * @date 2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * UNO Restriction: ARDUINO_AVR_UNO builds get only 5 objects
 * (AV:92-95, AV:99). AV:0-3 (ADC) and AV:96-98 (diagnostics) are excluded.
 * Enforced by static_assert with BACNET_AV_UNO_OBJECT_LIMIT.
 */

#ifndef BACNET_AV_H
#define BACNET_AV_H

#include <stdint.h>
#include <stdbool.h>

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "../platform/BACnetHardware.h"
#include "../platform/BACnetStorage.h"
#include "../bacnet/basic/object/av.h"

class BACnetAV {
public:
#if defined(ARDUINO_AVR_UNO)
    static constexpr uint8_t BACNET_AV_UNO_OBJECT_LIMIT = 5;
#endif

    static void     init();
    static bool     validInstance(uint32_t instance);
    static unsigned count();
    static uint32_t indexToInstance(unsigned idx);
    static unsigned instanceToIndex(uint32_t instance);
    static float    presentValue(uint32_t instance);
    static bool     presentValueSet(uint32_t instance, float value, uint8_t priority);
    static const char *nameASCII(uint32_t instance);
    static bool     nameSet(uint32_t instance, const char *value);
    static BACNET_ENGINEERING_UNITS units(uint32_t instance);
    static bool     unitsSet(uint32_t instance, BACNET_ENGINEERING_UNITS u);
    static int      readProperty(BACNET_READ_PROPERTY_DATA *rpdata);
    static bool     writeProperty(BACNET_WRITE_PROPERTY_DATA *wp_data);

private:
    struct ObjectData {
        const uint8_t object_id;
        const char *object_name;
        BACNET_ENGINEERING_UNITS units;
        float (*read_callback)(void);
        bool  (*write_callback)(float);
        float present_value;
    };

    static ObjectData _objects[];
    static const unsigned _objectsMax;

    static ObjectData *objectElement(uint32_t instance);

    /* read callbacks */
#if !defined(ARDUINO_AVR_UNO)
    static float adc0Value();
    static float adc1Value();
    static float adc2Value();
    static float adc3Value();
    static float mcuFrequencyFloat();
    static float stackSizeValue();
    static float stackUnusedValue();
#endif
    static float mstpBaud();
    static float mstpMac();
    static float mstpManager();
    static float deviceId();

    /* write callbacks */
    static bool mstpBaudWrite(float value);
    static bool mstpMacWrite(float value);
    static bool mstpManagerWrite(float value);
    static bool deviceIdWrite(float value);
};

#endif /* BACNET_AV_H */
