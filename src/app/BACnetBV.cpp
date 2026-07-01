/**
 * @file BACnetBV.cpp
 * @brief Binary Value BACnet objects — C++ class implementation
 * @author George Arun
 * @date 2026
 * @copyright SPDX-License-Identifier: MIT
 *
 */

#include "BACnetBV.h"
#include "../platform/BACnetHardware.h"
#include "../bacnet/bacdef.h"
#include "../bacnet/bacdcode.h"
#include "../bacnet/bacenum.h"
#include "../bacnet/bacapp.h"
#include "../bacnet/rp.h"
#include "../bacnet/wp.h"
#include "../bacnet/basic/object/bv.h"

/* =========================================================================
   Object table — 6 entries, identical to base bv.c
   ========================================================================= */
BACnetBV::ObjectData BACnetBV::_objects[BACnetBV::OBJECTS_MAX] = {
    {  0, "D3",  PIN_D3,  true },
    {  1, "D4",  PIN_D4,  true },
    {  2, "D5",  PIN_D5,  true },
    {  3, "D6",  PIN_D6,  true },
    {  4, "D7",  PIN_D7,  true },
    { 99, "LED", PIN_LED, true  }
};

BACNET_BINARY_PV BACnetBV::_presentValue[BACnetBV::OBJECTS_MAX];
bool             BACnetBV::_outOfService[BACnetBV::OBJECTS_MAX];

/* =========================================================================
   Private helpers
   ========================================================================= */

BACnetBV::ObjectData *BACnetBV::objectElement(uint32_t instance)
{
    for (unsigned i = 0; i < OBJECTS_MAX; i++) {
        if (_objects[i].object_id == instance) {
            return &_objects[i];
        }
    }
    return NULL;
}

/* =========================================================================
   Public API
   ========================================================================= */

void BACnetBV::init()
{
    for (unsigned i = 0; i < OBJECTS_MAX; i++) {
        _presentValue[i]  = BINARY_INACTIVE;
        _outOfService[i]  = false;
        BACnetHardware::pinInit(_objects[i].pin, _objects[i].is_output);
        if (_objects[i].is_output) {
            BACnetHardware::pinWrite(_objects[i].pin, false);
        }
    }
}

bool BACnetBV::validInstance(uint32_t instance)
{
    return objectElement(instance) != NULL;
}

unsigned BACnetBV::count()
{
    return OBJECTS_MAX;
}

uint32_t BACnetBV::indexToInstance(unsigned idx)
{
    if (idx < OBJECTS_MAX) {
        return _objects[idx].object_id;
    }
    return BACNET_MAX_INSTANCE;
}

unsigned BACnetBV::instanceToIndex(uint32_t instance)
{
    for (unsigned i = 0; i < OBJECTS_MAX; i++) {
        if (_objects[i].object_id == instance) {
            return i;
        }
    }
    return OBJECTS_MAX;
}

const char *BACnetBV::nameASCII(uint32_t instance)
{
    ObjectData *obj = objectElement(instance);
    return obj ? obj->object_name : "BV-X";
}

BACNET_BINARY_PV BACnetBV::presentValue(uint32_t instance)
{
    unsigned idx = instanceToIndex(instance);
    if (idx >= OBJECTS_MAX) {
        return BINARY_INACTIVE;
    }
    return _presentValue[idx];
}

bool BACnetBV::presentValueSet(uint32_t instance, BACNET_BINARY_PV value)
{
    unsigned idx   = instanceToIndex(instance);
    ObjectData *obj = objectElement(instance);
    if (!obj || !obj->is_output || idx >= OBJECTS_MAX) {
        return false;
    }
    _presentValue[idx] = value;
    BACnetHardware::pinWrite(obj->pin, (value == BINARY_ACTIVE));
    return true;
}

int BACnetBV::readProperty(BACNET_READ_PROPERTY_DATA *rpdata)
{
    uint8_t *apdu = rpdata->application_data;
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;

    unsigned idx = instanceToIndex(rpdata->object_instance);
    if (idx >= OBJECTS_MAX) {
        rpdata->error_class = ERROR_CLASS_OBJECT;
        rpdata->error_code  = ERROR_CODE_UNKNOWN_OBJECT;
        return BACNET_STATUS_ERROR;
    }

    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            return encode_application_object_id(apdu, OBJECT_BINARY_VALUE,
                                                rpdata->object_instance);
        case PROP_OBJECT_NAME:
            characterstring_init_ansi(&char_string, nameASCII(rpdata->object_instance));
            return encode_application_character_string(apdu, &char_string);
        case PROP_OBJECT_TYPE:
            return encode_application_enumerated(apdu, OBJECT_BINARY_VALUE);
        case PROP_PRESENT_VALUE:
            return encode_application_enumerated(apdu, _presentValue[idx]);
        case PROP_STATUS_FLAGS:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM,      false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT,         false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN,    false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE, _outOfService[idx]);
            return encode_application_bitstring(apdu, &bit_string);
        case PROP_EVENT_STATE:
            return encode_application_enumerated(apdu, EVENT_STATE_NORMAL);
        case PROP_OUT_OF_SERVICE:
            return encode_application_boolean(apdu, _outOfService[idx]);
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code  = ERROR_CODE_UNKNOWN_PROPERTY;
            return BACNET_STATUS_ERROR;
    }
}

bool BACnetBV::writeProperty(BACNET_WRITE_PROPERTY_DATA *wp_data)
{
    BACNET_APPLICATION_DATA_VALUE value;
    unsigned idx = instanceToIndex(wp_data->object_instance);

    if (idx >= OBJECTS_MAX) {
        wp_data->error_class = ERROR_CLASS_OBJECT;
        wp_data->error_code  = ERROR_CODE_UNKNOWN_OBJECT;
        return false;
    }

    if (wp_data->array_index != BACNET_ARRAY_ALL) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code  = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }

    if (bacapp_decode_application_data(wp_data->application_data,
                                       wp_data->application_data_len,
                                       &value) < 0) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code  = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }

    switch (wp_data->object_property) {
        case PROP_OUT_OF_SERVICE:
            if (value.tag != BACNET_APPLICATION_TAG_BOOLEAN) {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code  = ERROR_CODE_INVALID_DATA_TYPE;
                return false;
            }
            _outOfService[idx] = value.type.Boolean;
            return true;

        case PROP_PRESENT_VALUE:
            if (value.tag != BACNET_APPLICATION_TAG_ENUMERATED ||
                (value.type.Enumerated != BINARY_ACTIVE &&
                 value.type.Enumerated != BINARY_INACTIVE)) {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code  = ERROR_CODE_VALUE_OUT_OF_RANGE;
                return false;
            }
            return presentValueSet(wp_data->object_instance,
                                   (BACNET_BINARY_PV)value.type.Enumerated);

        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code  = ERROR_CODE_WRITE_ACCESS_DENIED;
            return false;
    }
}

/* =========================================================================
   extern "C" wrappers — satisfy bv.h C function signatures required by
   device.c, h_rp.c, h_wp.c (unchanged through Step 6)
   ========================================================================= */
extern "C" {

void Binary_Value_Init(void)
{
    BACnetBV::init();
}

bool Binary_Value_Valid_Instance(uint32_t instance)
{
    return BACnetBV::validInstance(instance);
}

unsigned Binary_Value_Count(void)
{
    return BACnetBV::count();
}

uint32_t Binary_Value_Index_To_Instance(unsigned index)
{
    return BACnetBV::indexToInstance(index);
}

unsigned Binary_Value_Instance_To_Index(uint32_t instance)
{
    return BACnetBV::instanceToIndex(instance);
}

const char *Binary_Value_Name_ASCII(uint32_t instance)
{
    return BACnetBV::nameASCII(instance);
}

BACNET_BINARY_PV Binary_Value_Present_Value(uint32_t instance)
{
    return BACnetBV::presentValue(instance);
}

bool Binary_Value_Present_Value_Set(uint32_t instance, BACNET_BINARY_PV value)
{
    return BACnetBV::presentValueSet(instance, value);
}

int Binary_Value_Read_Property(BACNET_READ_PROPERTY_DATA *rpdata)
{
    return BACnetBV::readProperty(rpdata);
}

bool Binary_Value_Write_Property(BACNET_WRITE_PROPERTY_DATA *wp_data)
{
    return BACnetBV::writeProperty(wp_data);
}

} /* extern "C" */
