/**
 * @file BACnetAV.cpp
 * @brief Analog Value BACnet objects — C++ class implementation
 * @author George Arun
 * @date 2026
 * @copyright SPDX-License-Identifier: MIT
 */

#include "BACnetAV.h"
#include "../platform/nvdata.h"
#include "../platform/rs485.h"
#include "../bacnet/bacdef.h"
#include "../bacnet/bacdcode.h"
#include "../bacnet/bacenum.h"
#include "../bacnet/bacapp.h"
#include "../bacnet/wp.h"
#include "../bacnet/basic/object/av.h"

/* forward declaration — defined in device.c */
extern "C" bool Device_Set_Object_Instance_Number(uint32_t object_id);

/* =========================================================================
   Private callback implementations
   ========================================================================= */

#if !defined(ARDUINO_AVR_UNO)
float BACnetAV::adc0Value()   { return (float)BACnetHardware::adcMillivolts(0); }
float BACnetAV::adc1Value()   { return (float)BACnetHardware::adcMillivolts(1); }
float BACnetAV::adc2Value()   { return (float)BACnetHardware::adcMillivolts(2); }
float BACnetAV::adc3Value()   { return (float)BACnetHardware::adcMillivolts(3); }
float BACnetAV::stackSizeValue()   { return (float)BACnetHardware::stackSize(); }
float BACnetAV::stackUnusedValue() { return (float)BACnetHardware::stackUnused(); }

float BACnetAV::mcuFrequencyFloat()
{
#if defined(ARDUINO_ARCH_ESP32)
    return (float)getCpuFrequencyMhz() * 1000000.0f;
#elif defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_ARCH_STM32F1) || \
      defined(ARDUINO_ARCH_STM32F4)
    extern uint32_t SystemCoreClock;
    return (float)SystemCoreClock;
#else
    return (float)clockCyclesPerMicrosecond() * 1000000.0f;
#endif
}
#endif /* !ARDUINO_AVR_UNO */

float BACnetAV::mstpBaud()
{
    uint8_t kbaud = BACnetStorage::unsigned8(NV_EEPROM_MSTP_BAUD_K);
    return (float)RS485_Baud_Rate_From_Kilo(kbaud);
}

bool BACnetAV::mstpBaudWrite(float value)
{
    int32_t baud = (int32_t)value;
    if (baud >= 9600L && baud <= 1152000L) {
        BACnetStorage::unsigned8_set(NV_EEPROM_MSTP_BAUD_K, (uint8_t)(baud / 1000UL));
        return true;
    }
    return false;
}

float BACnetAV::mstpMac()
{
    return (float)BACnetStorage::unsigned8(NV_EEPROM_MSTP_MAC);
}

bool BACnetAV::mstpMacWrite(float value)
{
    int32_t v = (int32_t)value;
    if (v >= 0L && v <= 127L) {
        BACnetStorage::unsigned8_set(NV_EEPROM_MSTP_MAC, (uint8_t)v);
        return true;
    }
    return false;
}

float BACnetAV::mstpManager()
{
    return (float)BACnetStorage::unsigned8(NV_EEPROM_MSTP_MAX_MASTER);
}

bool BACnetAV::mstpManagerWrite(float value)
{
    int32_t v = (int32_t)value;
    if (v >= 0 && v <= 127) {
        BACnetStorage::unsigned8_set(NV_EEPROM_MSTP_MAX_MASTER, (uint8_t)v);
        return true;
    }
    return false;
}

float BACnetAV::deviceId()
{
    return (float)BACnetStorage::unsigned24(NV_EEPROM_DEVICE_0);
}

bool BACnetAV::deviceIdWrite(float value)
{
    int32_t v = (int32_t)value;
    if (v >= 0 && v <= (int32_t)BACNET_MAX_INSTANCE) {
        BACnetStorage::unsigned24_set(NV_EEPROM_DEVICE_0, (uint32_t)v);
        Device_Set_Object_Instance_Number((uint32_t)v); /* GAP1 fix: live sync */
        return true;
    }
    return false;
}

/* =========================================================================
   Object table — UNO gets 5 entries, all other boards get 12
   ========================================================================= */
/* clang-format off */
BACnetAV::ObjectData BACnetAV::_objects[] = {
#if !defined(ARDUINO_AVR_UNO)
    {  0, "ADC0",             UNITS_MILLIVOLTS,      BACnetAV::adc0Value,       NULL,                       0.0f },
    {  1, "ADC1",             UNITS_MILLIVOLTS,      BACnetAV::adc1Value,       NULL,                       0.0f },
    {  2, "ADC2",             UNITS_MILLIVOLTS,      BACnetAV::adc2Value,       NULL,                       0.0f },
    {  3, "ADC3",             UNITS_MILLIVOLTS,      BACnetAV::adc3Value,       NULL,                       0.0f },
#endif
    { 92, "Device ID",        UNITS_NO_UNITS,        BACnetAV::deviceId,        BACnetAV::deviceIdWrite,    0.0f },
    { 93, "MS/TP Baud",       UNITS_BITS_PER_SECOND, BACnetAV::mstpBaud,        BACnetAV::mstpBaudWrite,    0.0f },
    { 94, "MS/TP MAC",        UNITS_NO_UNITS,        BACnetAV::mstpMac,         BACnetAV::mstpMacWrite,     0.0f },
    { 95, "MS/TP Max Manager",UNITS_NO_UNITS,        BACnetAV::mstpManager,     BACnetAV::mstpManagerWrite, 0.0f },
#if !defined(ARDUINO_AVR_UNO)
    { 96, "MCU Frequency",    UNITS_HERTZ,           BACnetAV::mcuFrequencyFloat, NULL,                     0.0f },
    { 97, "CStack Size",      UNITS_NO_UNITS,        BACnetAV::stackSizeValue,  NULL,                       0.0f },
    { 98, "CStack Unused",    UNITS_NO_UNITS,        BACnetAV::stackUnusedValue, NULL,                      0.0f },
#endif
    { 99, "Uptime",           UNITS_HOURS,           NULL,                      NULL,                       0.0f },
};
/* clang-format on */

const unsigned BACnetAV::_objectsMax =
    sizeof(BACnetAV::_objects) / sizeof(BACnetAV::_objects[0]);

/* =========================================================================
   Private helpers
   ========================================================================= */

BACnetAV::ObjectData *BACnetAV::objectElement(uint32_t instance)
{
    for (unsigned i = 0; i < _objectsMax; i++) {
        if (_objects[i].object_id == instance) {
            return &_objects[i];
        }
    }
    return NULL;
}

/* =========================================================================
   Public API
   ========================================================================= */

void BACnetAV::init()
{
#if defined(ARDUINO_AVR_UNO)
    static_assert(
        sizeof(_objects) / sizeof(_objects[0]) <= BACNET_AV_UNO_OBJECT_LIMIT,
        "UNO Restriction: AV object count exceeds BACNET_AV_UNO_OBJECT_LIMIT. "
        "Edit BACnetAV.h/cpp intentionally to raise the limit."
    );
#endif
#if !defined(ARDUINO_AVR_UNO)
    for (uint8_t i = 0; i < _objectsMax; i++) {
        BACnetHardware::adcEnable(i);
    }
#endif
}

bool BACnetAV::validInstance(uint32_t instance)
{
    return objectElement(instance) != NULL;
}

unsigned BACnetAV::count()
{
    return _objectsMax;
}

uint32_t BACnetAV::indexToInstance(unsigned idx)
{
    if (idx < _objectsMax) {
        return _objects[idx].object_id;
    }
    return UINT32_MAX;
}

unsigned BACnetAV::instanceToIndex(uint32_t instance)
{
    unsigned index;
    for (index = 0; index < _objectsMax; index++) {
        if (_objects[index].object_id == instance) {
            break;
        }
    }
    return index;
}

float BACnetAV::presentValue(uint32_t instance)
{
    ObjectData *obj = objectElement(instance);
    if (obj) {
        return obj->read_callback ? obj->read_callback() : obj->present_value;
    }
    return 0.0f;
}

bool BACnetAV::presentValueSet(uint32_t instance, float value, uint8_t priority)
{
    (void)priority;
    ObjectData *obj = objectElement(instance);
    if (obj) {
        if (obj->write_callback) {
            return obj->write_callback(value);
        } else {
            obj->present_value = value;
            return true;
        }
    }
    return false;
}

const char *BACnetAV::nameASCII(uint32_t instance)
{
    ObjectData *obj = objectElement(instance);
    return obj ? obj->object_name : "AV-X";
}

bool BACnetAV::nameSet(uint32_t instance, const char *value)
{
    ObjectData *obj = objectElement(instance);
    if (obj) {
        obj->object_name = value;
        return true;
    }
    return false;
}

BACNET_ENGINEERING_UNITS BACnetAV::units(uint32_t instance)
{
    ObjectData *obj = objectElement(instance);
    return obj ? obj->units : UNITS_NO_UNITS;
}

bool BACnetAV::unitsSet(uint32_t instance, BACNET_ENGINEERING_UNITS u)
{
    ObjectData *obj = objectElement(instance);
    if (obj) {
        obj->units = u;
        return true;
    }
    return false;
}

int BACnetAV::readProperty(BACNET_READ_PROPERTY_DATA *rpdata)
{
    int apdu_len = 0;
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    uint8_t *apdu = rpdata->application_data;

    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len = encode_application_object_id(
                &apdu[0], OBJECT_ANALOG_VALUE, rpdata->object_instance);
            break;
        case PROP_OBJECT_NAME:
            characterstring_init_ansi(&char_string, nameASCII(rpdata->object_instance));
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len = encode_application_enumerated(&apdu[0], OBJECT_ANALOG_VALUE);
            break;
        case PROP_PRESENT_VALUE:
            apdu_len = encode_application_real(&apdu[0], presentValue(rpdata->object_instance));
            break;
        case PROP_STATUS_FLAGS:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE, false);
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_EVENT_STATE:
            apdu_len = encode_application_enumerated(&apdu[0], EVENT_STATE_NORMAL);
            break;
        case PROP_OUT_OF_SERVICE:
            apdu_len = encode_application_boolean(&apdu[0], false);
            break;
        case PROP_UNITS:
            apdu_len = encode_application_enumerated(&apdu[0], units(rpdata->object_instance));
            break;
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }

    if ((apdu_len >= 0) && (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}

bool BACnetAV::writeProperty(BACNET_WRITE_PROPERTY_DATA *wp_data)
{
    bool status = false;
    int len = 0;
    BACNET_APPLICATION_DATA_VALUE value = { 0 };

    if (!validInstance(wp_data->object_instance)) {
        wp_data->error_class = ERROR_CLASS_OBJECT;
        wp_data->error_code = ERROR_CODE_UNKNOWN_OBJECT;
        return false;
    }

    len = bacapp_decode_application_data(
        wp_data->application_data, wp_data->application_data_len, &value);
    if (len < 0) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }
    if ((wp_data->object_property != PROP_PRIORITY_ARRAY) &&
        (wp_data->array_index != BACNET_ARRAY_ALL)) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }

    switch (wp_data->object_property) {
        case PROP_PRESENT_VALUE:
            if (value.tag == BACNET_APPLICATION_TAG_REAL) {
                status = presentValueSet(wp_data->object_instance, value.type.Real,
                                         wp_data->priority);
                if (!status) {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }
            break;
        case PROP_UNITS:
            if (value.tag == BACNET_APPLICATION_TAG_ENUMERATED) {
                if (value.type.Enumerated <= UINT16_MAX) {
                    unitsSet(wp_data->object_instance,
                             (BACNET_ENGINEERING_UNITS)value.type.Enumerated);
                    status = true;
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }
            break;
        case PROP_OBJECT_IDENTIFIER:
        case PROP_OBJECT_NAME:
        case PROP_OBJECT_TYPE:
        case PROP_STATUS_FLAGS:
        case PROP_EVENT_STATE:
        case PROP_OUT_OF_SERVICE:
        case PROP_DESCRIPTION:
        case PROP_PRIORITY_ARRAY:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            break;
        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            break;
    }

    return status;
}

/* =========================================================================
   extern "C" wrappers — satisfy av.h C function signatures required by
   device.c, h_rp.c, h_wp.c (unchanged from base project)
   ========================================================================= */
extern "C" {

void Analog_Value_Init(void)
{
    BACnetAV::init();
}

bool Analog_Value_Valid_Instance(uint32_t object_instance)
{
    return BACnetAV::validInstance(object_instance);
}

unsigned Analog_Value_Count(void)
{
    return BACnetAV::count();
}

uint32_t Analog_Value_Index_To_Instance(unsigned index)
{
    return BACnetAV::indexToInstance(index);
}

unsigned Analog_Value_Instance_To_Index(uint32_t object_instance)
{
    return BACnetAV::instanceToIndex(object_instance);
}

float Analog_Value_Present_Value(uint32_t object_instance)
{
    return BACnetAV::presentValue(object_instance);
}

bool Analog_Value_Present_Value_Set(uint32_t object_instance, float value,
                                    uint8_t priority)
{
    return BACnetAV::presentValueSet(object_instance, value, priority);
}

const char *Analog_Value_Name_ASCII(uint32_t object_instance)
{
    return BACnetAV::nameASCII(object_instance);
}

bool Analog_Value_Name_Set(uint32_t object_instance, const char *value)
{
    return BACnetAV::nameSet(object_instance, value);
}

BACNET_ENGINEERING_UNITS Analog_Value_Units(uint32_t object_instance)
{
    return BACnetAV::units(object_instance);
}

bool Analog_Value_Units_Set(uint32_t object_instance,
                            BACNET_ENGINEERING_UNITS u)
{
    return BACnetAV::unitsSet(object_instance, u);
}

int Analog_Value_Read_Property(BACNET_READ_PROPERTY_DATA *rpdata)
{
    return BACnetAV::readProperty(rpdata);
}

bool Analog_Value_Write_Property(BACNET_WRITE_PROPERTY_DATA *wp_data)
{
    return BACnetAV::writeProperty(wp_data);
}

} /* extern "C" */
