

/**
 * @file bv.c
 * @brief Binary Value BACnet objects — Arduino implementation
 * @author George Arun
 * @date 2025-2026
 * @copyright SPDX-License-Identifier: MIT
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "../platform/pin_config.h"

#include "../bacnet/bacdef.h"
#include "../bacnet/bacdcode.h"
#include "../bacnet/bacenum.h"
#include "../bacnet/config.h"
#include "../bacnet/rp.h"
#include "../bacnet/wp.h"
#include "../bacnet/basic/object/bv.h"

/* -----------------------------------------
   Object structure
   ----------------------------------------- */
 #ifdef __cplusplus
extern "C" {
#endif    


struct object_data {
    const uint8_t object_id;
    const char *object_name;
    uint8_t pin;
    bool is_output;
};

/* -----------------------------------------
   Object list
   ----------------------------------------- */
static struct object_data Object_List[] = {
    { 0,  "D3",  PIN_D3,  true },
    { 1,  "D4",  PIN_D4,  true },
    { 2,  "D5",  PIN_D5,  true },
    { 3,  "D6",  PIN_D6,  true },
    { 4,  "D7",  PIN_D7,  true },
    { 99, "LED", PIN_LED, true  }
};

/* number of objects */
#define OBJECTS_MAX (sizeof(Object_List) / sizeof(Object_List[0]))

static BACNET_BINARY_PV Present_Value[OBJECTS_MAX];
static bool Out_Of_Service[OBJECTS_MAX];

/* -----------------------------------------
   Helpers
   ----------------------------------------- */
static struct object_data *Object_List_Element(uint32_t instance)
{
    for (unsigned i = 0; i < OBJECTS_MAX; i++) {
        if (Object_List[i].object_id == instance) {
            return &Object_List[i];
        }
    }
    return NULL;
}

unsigned Binary_Value_Instance_To_Index(uint32_t instance)
{
    for (unsigned i = 0; i < OBJECTS_MAX; i++) {
        if (Object_List[i].object_id == instance) {
            return i;
        }
    }
    return OBJECTS_MAX;
}

/* -----------------------------------------
   BACnet API
   ----------------------------------------- */
bool Binary_Value_Valid_Instance(uint32_t instance)
{
    return (Binary_Value_Instance_To_Index(instance) < OBJECTS_MAX);
}

unsigned Binary_Value_Count(void)
{
    return OBJECTS_MAX;
}

uint32_t Binary_Value_Index_To_Instance(unsigned index)
{
    if (index < OBJECTS_MAX) {
        return Object_List[index].object_id;
    }
    return BACNET_MAX_INSTANCE;
}

const char *Binary_Value_Name_ASCII(uint32_t instance)
{
    struct object_data *obj = Object_List_Element(instance);
    return obj ? obj->object_name : "BV-X";
}

/* -----------------------------------------
   Present Value
   ----------------------------------------- */
BACNET_BINARY_PV Binary_Value_Present_Value(uint32_t instance)
{
    unsigned index = Binary_Value_Instance_To_Index(instance);
    if (index >= OBJECTS_MAX) {
        return BINARY_INACTIVE;
    }
    return Present_Value[index];
}

bool Binary_Value_Present_Value_Set(uint32_t instance, BACNET_BINARY_PV value)
{
    unsigned index = Binary_Value_Instance_To_Index(instance);
    struct object_data *obj = Object_List_Element(instance);

    if (!obj || !obj->is_output || index >= OBJECTS_MAX) {
        return false;
    }

    Present_Value[index] = value;
    PIN_WRITE(obj->pin, (value == BINARY_ACTIVE));
    return true;
}

/* -----------------------------------------
   Read Property
   ----------------------------------------- */
int Binary_Value_Read_Property(BACNET_READ_PROPERTY_DATA *rpdata)
{
    uint8_t *apdu = rpdata->application_data;
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;

    unsigned index = Binary_Value_Instance_To_Index(rpdata->object_instance);
    if (index >= OBJECTS_MAX) {
        rpdata->error_class = ERROR_CLASS_OBJECT;
        rpdata->error_code = ERROR_CODE_UNKNOWN_OBJECT;
        return BACNET_STATUS_ERROR;
    }

    switch (rpdata->object_property) {

        case PROP_OBJECT_IDENTIFIER:
            return encode_application_object_id(apdu, OBJECT_BINARY_VALUE,
                                                rpdata->object_instance);

        case PROP_OBJECT_NAME:
            characterstring_init_ansi(&char_string,
                                      Binary_Value_Name_ASCII(rpdata->object_instance));
            return encode_application_character_string(apdu, &char_string);

        case PROP_OBJECT_TYPE:
            return encode_application_enumerated(apdu, OBJECT_BINARY_VALUE);

        case PROP_PRESENT_VALUE:
            return encode_application_enumerated(apdu, Present_Value[index]);

        case PROP_STATUS_FLAGS:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE,
                              Out_Of_Service[index]);
            return encode_application_bitstring(apdu, &bit_string);

        case PROP_EVENT_STATE:
            return encode_application_enumerated(apdu, EVENT_STATE_NORMAL);

        case PROP_OUT_OF_SERVICE:
            return encode_application_boolean(apdu, Out_Of_Service[index]);

        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            return BACNET_STATUS_ERROR;
    }
}

/* -----------------------------------------
   Write Property
   ----------------------------------------- */
bool Binary_Value_Write_Property(BACNET_WRITE_PROPERTY_DATA *wp)
{
    BACNET_APPLICATION_DATA_VALUE value;
    unsigned index = Binary_Value_Instance_To_Index(wp->object_instance);

    if (index >= OBJECTS_MAX) {
        wp->error_class = ERROR_CLASS_OBJECT;
        wp->error_code = ERROR_CODE_UNKNOWN_OBJECT;
        return false;
    }

    if (wp->array_index != BACNET_ARRAY_ALL) {
        wp->error_class = ERROR_CLASS_PROPERTY;
        wp->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }

    if (bacapp_decode_application_data(wp->application_data,
                                       wp->application_data_len,
                                       &value) < 0) {
        wp->error_class = ERROR_CLASS_PROPERTY;
        wp->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }

    switch (wp->object_property) {

        case PROP_OUT_OF_SERVICE:
            if (value.tag != BACNET_APPLICATION_TAG_BOOLEAN) {
                wp->error_class = ERROR_CLASS_PROPERTY;
                wp->error_code = ERROR_CODE_INVALID_DATA_TYPE;
                return false;
            }
            Out_Of_Service[index] = value.type.Boolean;
            return true;

        case PROP_PRESENT_VALUE:
            if (value.tag != BACNET_APPLICATION_TAG_ENUMERATED ||
                (value.type.Enumerated != BINARY_ACTIVE &&
                 value.type.Enumerated != BINARY_INACTIVE)) {

                wp->error_class = ERROR_CLASS_PROPERTY;
                wp->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                return false;
            }

            return Binary_Value_Present_Value_Set(
                wp->object_instance,
                (BACNET_BINARY_PV)value.type.Enumerated);

        default:
            wp->error_class = ERROR_CLASS_PROPERTY;
            wp->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            return false;
    }
}


/* -----------------------------------------
   Init
   ----------------------------------------- */
void Binary_Value_Init(void)
{
    for (unsigned i = 0; i < OBJECTS_MAX; i++) {
        Present_Value[i] = BINARY_INACTIVE;
        Out_Of_Service[i] = false;

        PIN_INIT(Object_List[i].pin, Object_List[i].is_output);
        if (Object_List[i].is_output) {
            PIN_WRITE(Object_List[i].pin, false);
        }
    }
}

#ifdef __cplusplus
}
#endif

