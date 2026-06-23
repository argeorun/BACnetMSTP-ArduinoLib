/**
 * @file BACnetBinaryValue.cpp
 * @brief BACnet Binary Value object C++ wrapper — implementation.
 * @author George Arun
 * @date 2025-2026
 * @copyright SPDX-License-Identifier: MIT
 */

#if !defined(ARDUINO)
#error "BACnetBinaryValue.cpp is Arduino-only; do not include it in desktop builds."
#endif

#include "BACnetBinaryValue.h"

/* Compile-time safety check: BACnetBinaryValue::State must map to BACNET_BINARY_PV */
static_assert((int)BACnetBinaryValue::INACTIVE == (int)BINARY_INACTIVE,
    "BACnetBinaryValue::INACTIVE must equal BINARY_INACTIVE");
static_assert((int)BACnetBinaryValue::ACTIVE == (int)BINARY_ACTIVE,
    "BACnetBinaryValue::ACTIVE must equal BINARY_ACTIVE");

BACnetBinaryValue::BACnetBinaryValue()
    : BACnetObject(OBJECT_BINARY_VALUE)
{
    Binary_Value_Init();
}

BACnetBinaryValue::State BACnetBinaryValue::getState(uint32_t instance)
{
    BACNET_BINARY_PV pv = Binary_Value_Present_Value(instance);
    return (pv == BINARY_ACTIVE) ? ACTIVE : INACTIVE;
}

bool BACnetBinaryValue::setState(uint32_t instance, State s)
{
    if (!Binary_Value_Is_Output(instance)) {
        return false;
    }
    return Binary_Value_Present_Value_Set(instance,
                                          (s == ACTIVE) ? BINARY_ACTIVE
                                                        : BINARY_INACTIVE);
}

uint8_t BACnetBinaryValue::getPin(uint32_t instance)
{
    return Binary_Value_Pin(instance);
}

bool BACnetBinaryValue::isOutput(uint32_t instance)
{
    return Binary_Value_Is_Output(instance);
}

unsigned BACnetBinaryValue::count() const
{
    return Binary_Value_Count();
}

int BACnetBinaryValue::readProperty(BACNET_READ_PROPERTY_DATA *rpdata)
{
    return Binary_Value_Read_Property(rpdata);
}

bool BACnetBinaryValue::writeProperty(BACNET_WRITE_PROPERTY_DATA *wpdata)
{
    return Binary_Value_Write_Property(wpdata);
}

bool BACnetBinaryValue::validInstance(uint32_t id) const
{
    return Binary_Value_Valid_Instance(id);
}
