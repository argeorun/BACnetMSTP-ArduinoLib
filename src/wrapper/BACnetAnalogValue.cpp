/**
 * @file BACnetAnalogValue.cpp
 * @brief BACnet Analog Value object C++ wrapper — implementation.
 * @author George Arun
 * @date 2025-2026
 * @copyright SPDX-License-Identifier: MIT
 */

#if !defined(ARDUINO)
#error "BACnetAnalogValue.cpp is Arduino-only; do not include it in desktop builds."
#endif

#include "BACnetAnalogValue.h"

/* Compile-time safety checks */
static_assert(sizeof(float) == 4,
    "BACnetAnalogValue requires 32-bit float (IEEE 754)");

BACnetAnalogValue::BACnetAnalogValue()
    : BACnetObject(OBJECT_ANALOG_VALUE)
{
    Analog_Value_Init();
}

float BACnetAnalogValue::getPresentValue(uint32_t instance)
{
    return Analog_Value_Present_Value(instance);
}

bool BACnetAnalogValue::setPresentValue(uint32_t instance, float value,
                                        uint8_t priority)
{
    if (!Analog_Value_Valid_Instance(instance)) {
        return false;
    }
    return Analog_Value_Present_Value_Set(instance, value, priority);
}

uint16_t BACnetAnalogValue::getUnits(uint32_t instance)
{
    return (uint16_t)Analog_Value_Units(instance);
}

bool BACnetAnalogValue::setUnits(uint32_t instance, uint16_t units)
{
    if (!Analog_Value_Valid_Instance(instance)) {
        return false;
    }
    return Analog_Value_Units_Set(instance,
                                  (BACNET_ENGINEERING_UNITS)units);
}

bool BACnetAnalogValue::isWritable(uint32_t instance)
{
    /* All valid AV instances accept Present_Value writes in this implementation.
     * Out-of-Service control is handled inside av.c's Write_Property handler. */
    return Analog_Value_Valid_Instance(instance);
}

unsigned BACnetAnalogValue::count() const
{
    return Analog_Value_Count();
}

int BACnetAnalogValue::readProperty(BACNET_READ_PROPERTY_DATA *rpdata)
{
    return Analog_Value_Read_Property(rpdata);
}

bool BACnetAnalogValue::writeProperty(BACNET_WRITE_PROPERTY_DATA *wpdata)
{
    return Analog_Value_Write_Property(wpdata);
}

bool BACnetAnalogValue::validInstance(uint32_t id) const
{
    return Analog_Value_Valid_Instance(id);
}
