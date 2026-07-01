/**
 * @file BACnetDevice.cpp
 * @brief BACnet Device object + APDU/RP/WP/WhoIs handlers â€" C++ class implementation
 * @author George Arun
 * @date 2026
 * @copyright SPDX-License-Identifier: MIT
 *
 */

#include "BACnetDevice.h"
#include "BACnetAV.h"
#include "BACnetBV.h"
#include "../platform/BACnetStorage.h"
#include "../platform/BACnetHardware.h"
#include "../platform/nvdata.h"
#include "../platform/rs485.h"
#include "../platform/rs485_port_config.h"
#include "../compile_config.h"

/* BACnet stack */
#include "../bacnet/bacdef.h"
#include "../bacnet/bacdcode.h"
#include "../bacnet/bacstr.h"
#include "../bacnet/bacenum.h"
#include "../bacnet/bacapp.h"
#include "../bacnet/apdu.h"
#include "../bacnet/dcc.h"
#include "../bacnet/npdu.h"
#include "../bacnet/abort.h"
#include "../bacnet/bacerror.h"
#include "../bacnet/rp.h"
#include "../bacnet/wp.h"
#include "../bacnet/whois.h"
#include "../bacnet/iam.h"
#include "../bacnet/version.h"
#include "../bacnet/datalink/datalink.h"
#include "../bacnet/datalink/dlmstp.h"
#include "../bacnet/basic/services.h"
#include "../bacnet/basic/tsm/tsm.h"
#include "../bacnet/basic/sys/mstimer.h"
#include "../bacnet/basic/object/device.h"
#include "../bacnet/basic/object/av.h"
#include "../bacnet/basic/object/bv.h"

#ifdef ARDUINO
#  include <Arduino.h>
#endif

/* =========================================================================
   Debug macros — two categories controlled by compile_config.h:
     BACNET_DEBUG_BOOT    → banner, MAC, baud, stack at startup
     BACNET_DEBUG_TRAFFIC → [rx] [apdu] [rp] [wp] [whois] [tick] at runtime
   ========================================================================= */

/* Serial.begin() needed if either category is active */
#if defined(BACNET_DEBUG_BOOT) || defined(BACNET_DEBUG_TRAFFIC)
#  define DBG_BEGIN()     Serial.begin(115200)
#else
#  define DBG_BEGIN()
#endif

/* Boot macros */
#if defined(BACNET_DEBUG_BOOT)
#  define DBOOT(x)        Serial.print(x)
#  define DBOOTLN(x)      Serial.println(x)
#  define DBOOT2(x, f)    Serial.print(x, f)
#else
#  define DBOOT(x)
#  define DBOOTLN(x)
#  define DBOOT2(x, f)
#endif

/* Traffic macros */
#if defined(BACNET_DEBUG_TRAFFIC)
#  define DTFC(x)         Serial.print(x)
#  define DTFCLN(x)       Serial.println(x)
#  define DTFC2(x, f)     Serial.print(x, f)
#else
#  define DTFC(x)
#  define DTFCLN(x)
#  define DTFC2(x, f)
#endif

/* =========================================================================
   C-linkage globals â€" Send_I_Am_Flag referenced by dlmstp.c;
   BACnetDevice_NetworkPriority shared with apdu_shim.c (which defines
   apdu_network_priority / apdu_network_priority_set as C functions because
   apdu.h has no extern "C" guards and those two functions are called from
   the C file h_npdu.c â€" they cannot be defined here without a linkage clash)
   ========================================================================= */
extern "C" {
    bool    Send_I_Am_Flag             = true;
    uint8_t BACnetDevice_NetworkPriority = 0;
}

/* =========================================================================
   Static member definitions
   ========================================================================= */
uint32_t             BACnetDevice::_objectInstanceNumber = 260001;
BACNET_DEVICE_STATUS BACnetDevice::_systemStatus         = STATUS_OPERATIONAL;
BACNET_WRITE_PROPERTY_DATA BACnetDevice::_wpData;
uint32_t             BACnetDevice::_uptimeSeconds         = 0;
struct mstimer       BACnetDevice::_taskTimer;

static const char *_modelName = "ATmega328 Uno R3 Device";

/* =========================================================================
   AVProxy / BVProxy inline implementations
   ========================================================================= */
float AVProxy::get() const
{
    return BACnetAV::presentValue(_instance);
}

bool AVProxy::set(float value, uint8_t priority) const
{
    return BACnetAV::presentValueSet(_instance, value, priority);
}

BACNET_BINARY_PV BVProxy::get() const
{
    return BACnetBV::presentValue(_instance);
}

bool BVProxy::set(BACNET_BINARY_PV value) const
{
    return BACnetBV::presentValueSet(_instance, value);
}

bool BVProxy::set(bool active) const
{
    return BACnetBV::presentValueSet(_instance,
                                     active ? BINARY_ACTIVE : BINARY_INACTIVE);
}

/* =========================================================================
   Private: hardware init â€" mirrors base project hardware_init()
   ========================================================================= */
void BACnetDevice::hardwareInit()
{
    DBG_BEGIN();
    BACnetStorage::begin();
    RS485_Initialize();
    mstimer_init();
    adc_init();
#if defined(ARDUINO)
    interrupts();
#endif
}

/* =========================================================================
   Private: EEPROM/NV init â€" mirrors base project device_nvdata_init()
   ========================================================================= */
void BACnetDevice::deviceNvdataInit()
{
    uint8_t  value8;
    uint16_t value16;
    uint32_t value32;

    value16 = nvdata_unsigned16(NV_EEPROM_TYPE_0);
    DBOOT(F("[BACnet] EEPROM type tag: 0x"));
    DBOOT2(value16, HEX);
    DBOOT(F("  (expect 0x"));
    DBOOT2((uint16_t)NV_EEPROM_TYPE_ID, HEX);
    DBOOTLN(F(")"));

    if (value16 != NV_EEPROM_TYPE_ID) {
        DBOOTLN(F("[BACnet] FRESH EEPROM - writing defaults"));
        nvdata_unsigned16_set(NV_EEPROM_TYPE_0,        NV_EEPROM_TYPE_ID);
        nvdata_unsigned8_set(NV_EEPROM_VERSION,         NV_EEPROM_VERSION_ID);
        nvdata_unsigned8_set(NV_EEPROM_MSTP_MAC,        10);
        nvdata_unsigned8_set(NV_EEPROM_MSTP_BAUD_K,     38);
        nvdata_unsigned8_set(NV_EEPROM_MSTP_MAX_MASTER, 127);
        nvdata_unsigned24_set(NV_EEPROM_DEVICE_0,       260001);
        DBOOTLN(F("[BACnet] Defaults written"));
    } else {
        DBOOTLN(F("[BACnet] EXISTING EEPROM - reading stored values"));
    }

    value8 = nvdata_unsigned8(NV_EEPROM_MSTP_MAC);
    DBOOT(F("[BACnet] MAC="));
    DBOOTLN(value8);
    dlmstp_set_mac_address(value8);

    value8 = nvdata_unsigned8(NV_EEPROM_MSTP_BAUD_K);
    DBOOT(F("[BACnet] baud_k="));
    DBOOT(value8);
    value32 = RS485_Baud_Rate_From_Kilo(value8);
    DBOOT(F("  -> "));
    DBOOT(value32);
    DBOOTLN(F(" bps"));
    RS485_Set_Baud_Rate(value32);

    value8 = nvdata_unsigned8(NV_EEPROM_MSTP_MAX_MASTER);
    DBOOT(F("[BACnet] max_master="));
    DBOOTLN(value8);
    dlmstp_set_max_master(value8);

    dlmstp_set_max_info_frames(1);

    value32 = nvdata_unsigned24(NV_EEPROM_DEVICE_0);
    DBOOT(F("[BACnet] device_instance="));
    DBOOTLN(value32);
    _objectInstanceNumber = value32;
    Device_Set_Object_Instance_Number(value32);

    Send_I_Am_Flag = true;
    DBOOTLN(F("[BACnet] Send_I_Am_Flag set"));
}

/* =========================================================================
   Private: one-second task â€" mirrors base project one_second_task()
   ========================================================================= */
void BACnetDevice::oneSecondTask()
{
    _uptimeSeconds++;
    float hours = (float)_uptimeSeconds / 3600.0f;
    Analog_Value_Present_Value_Set(99, hours, 8);
    /* Send_I_Am_Flag is owned by dlmstp.c â€" dlmstp_encode_unconfirmed_frame
       checks and clears it when the device holds the token (lines 218-219).
       The base project one_second_task never calls Send_I_Am here. */
    if (_uptimeSeconds % 10 == 0) {
        DTFC(F("[tick] uptime="));
        DTFC(_uptimeSeconds);
        DTFC(F("s  hours="));
        DTFC(hours);
        DTFC(F("  Send_I_Am_Flag="));
        DTFCLN(Send_I_Am_Flag ? F("true") : F("false"));
    }
}

/* =========================================================================
   Public: begin() â€" call once in setup()
   ========================================================================= */
void BACnetDevice::begin()
{
    hardwareInit();

    DBOOTLN(F(""));
    DBOOTLN(F("=== BACnetMSTP-ArduinoLib v2.0.0 ==="));
#if defined(ARDUINO_AVR_MEGA2560)
    DBOOTLN(F("Board : Arduino Mega 2560"));
#elif defined(ARDUINO_AVR_UNO)
    /* Uno: macros expand to nothing */
#elif defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_IDF_TARGET_ESP32S3)
    DBOOTLN(F("Board : ESP32-S3"));
#elif defined(ARDUINO_ARCH_ESP32)
    DBOOTLN(F("Board : ESP32"));
#elif defined(ARDUINO_ARCH_STM32)
    DBOOTLN(F("Board : STM32"));
#else
    DBOOTLN(F("Board : Unknown"));
#endif
    DBOOTLN(F("======================================"));

    DBOOTLN(F("[BACnet] hardwareInit done"));

    DBOOT(F("[BACnet] RS485 port: "));
    DBOOTLN(F(RS485_PORT_NAME));

    DBOOT(F("[BACnet] adcInit done  channels="));
    DBOOTLN(BACnetHardware::adcChannelCount());

    DBOOT(F("[BACnet] stack size="));
    DBOOT(BACnetHardware::stackSize());
    DBOOT(F("  unused="));
    DBOOTLN(BACnetHardware::stackUnused());

    Analog_Value_Init();
    DBOOT(F("[BACnet] AV objects="));
    DBOOTLN(Analog_Value_Count());

    Binary_Value_Init();
    DBOOT(F("[BACnet] BV objects="));
    DBOOTLN(Binary_Value_Count());

    deviceNvdataInit();
    DBOOTLN(F("[BACnet] deviceNvdataInit done"));

    dlmstp_init(NULL);
    DBOOTLN(F("[BACnet] dlmstp_init done"));

    mstimer_set(&_taskTimer, 1000);
    DBOOTLN(F("[BACnet] BACnet stack ready - waiting for traffic"));
}

/* =========================================================================
   Public: update() â€" call every loop()
   ========================================================================= */
void BACnetDevice::update()
{
    static uint8_t PDUBuffer[MAX_MPDU + 16];
    uint16_t pdu_len;
    BACNET_ADDRESS src;

    if (mstimer_expired(&_taskTimer)) {
        mstimer_reset(&_taskTimer);
        oneSecondTask();
    }

    pdu_len = dlmstp_receive(&src, PDUBuffer, sizeof(PDUBuffer), 0);
    if (pdu_len) {
        DTFC(F("[rx] src=0x"));
        DTFC2(src.adr[0], HEX);
        DTFC(F("  len="));
        DTFCLN(pdu_len);
        npdu_handler(&src, PDUBuffer, pdu_len);
    }
}

/* =========================================================================
   Device object methods â€" mirrors device.c
   ========================================================================= */

uint32_t BACnetDevice::objectInstanceNumber()
{
    return _objectInstanceNumber;
}

bool BACnetDevice::setObjectInstanceNumber(uint32_t object_id)
{
    if (object_id <= BACNET_MAX_INSTANCE) {
        _objectInstanceNumber = object_id;
        return true;
    }
    return false;
}

bool BACnetDevice::validObjectInstanceNumber(uint32_t object_id)
{
    return (_objectInstanceNumber == object_id);
}

uint16_t BACnetDevice::vendorIdentifier()
{
    return BACNET_VENDOR_ID;
}

unsigned BACnetDevice::objectListCount()
{
    unsigned count = 1; /* device object */
    count += Analog_Value_Count();
    count += Binary_Value_Count();
    return count;
}

bool BACnetDevice::objectListIdentifier(
    uint32_t array_index, BACNET_OBJECT_TYPE *object_type, uint32_t *instance)
{
    bool status = false;
    uint32_t object_index = 0;
    uint32_t object_count = 0;

    if (array_index == 1) {
        *object_type = OBJECT_DEVICE;
        *instance    = _objectInstanceNumber;
        return true;
    }
    object_index = array_index - 1;
    object_count = 1;

    if (!status) {
        object_index -= object_count;
        object_count = Analog_Value_Count();
        if (object_index < object_count) {
            *object_type = OBJECT_ANALOG_VALUE;
            *instance    = Analog_Value_Index_To_Instance(object_index);
            status = true;
        }
    }
    if (!status) {
        object_index -= object_count;
        object_count = Binary_Value_Count();
        if (object_index < object_count) {
            *object_type = OBJECT_BINARY_VALUE;
            *instance    = Binary_Value_Index_To_Instance(object_index);
            status = true;
        }
    }
    return status;
}

int BACnetDevice::objectListElementEncode(
    uint32_t object_instance, BACNET_ARRAY_INDEX array_index, uint8_t *apdu)
{
    int apdu_len = BACNET_STATUS_ERROR;
    BACNET_OBJECT_TYPE object_type;
    uint32_t instance;

    if (object_instance == objectInstanceNumber()) {
        array_index++;
        if (objectListIdentifier(array_index, &object_type, &instance)) {
            apdu_len = encode_application_object_id(apdu, object_type, instance);
        }
    }
    return apdu_len;
}

int BACnetDevice::readProperty(BACNET_READ_PROPERTY_DATA *rpdata)
{
    uint8_t *apdu    = rpdata->application_data;
    int apdu_max     = rpdata->application_data_len;
    int apdu_len     = 0;
    uint32_t i       = 0;
    uint32_t count   = 0;
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;

    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len = encode_application_object_id(
                &apdu[0], OBJECT_DEVICE, _objectInstanceNumber);
            break;
        case PROP_OBJECT_NAME:
            nvdata_name(NV_EEPROM_DEVICE_NAME, &char_string, "BACnet Device");
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_DESCRIPTION:
            nvdata_name(NV_EEPROM_DEVICE_DESCRIPTION, &char_string, "Description");
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_LOCATION:
            nvdata_name(NV_EEPROM_DEVICE_LOCATION, &char_string, "Location");
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len = encode_application_enumerated(&apdu[0], OBJECT_DEVICE);
            break;
        case PROP_SYSTEM_STATUS:
            apdu_len = encode_application_enumerated(&apdu[0], _systemStatus);
            break;
        case PROP_VENDOR_NAME:
            characterstring_init_ansi(&char_string, BACNET_VENDOR_NAME);
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_VENDOR_IDENTIFIER:
            apdu_len = encode_application_unsigned(&apdu[0], vendorIdentifier());
            break;
        case PROP_MODEL_NAME:
            characterstring_init_ansi(&char_string, _modelName);
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_FIRMWARE_REVISION:
            characterstring_init_ansi(&char_string, BACNET_VERSION_TEXT);
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_APPLICATION_SOFTWARE_VERSION:
            characterstring_init_ansi(&char_string, "1.0");
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_PROTOCOL_VERSION:
            apdu_len = encode_application_unsigned(&apdu[0], BACNET_PROTOCOL_VERSION);
            break;
        case PROP_PROTOCOL_REVISION:
            apdu_len = encode_application_unsigned(&apdu[0], BACNET_PROTOCOL_REVISION);
            break;
        case PROP_PROTOCOL_SERVICES_SUPPORTED:
            bitstring_init(&bit_string);
            for (i = 0; i < MAX_BACNET_SERVICES_SUPPORTED; i++) {
                bitstring_set_bit(&bit_string, (uint8_t)i,
                    apduServiceSupported((BACNET_SERVICES_SUPPORTED)i));
            }
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_PROTOCOL_OBJECT_TYPES_SUPPORTED:
            bitstring_init(&bit_string);
            for (i = 0; i < MAX_ASHRAE_OBJECT_TYPE; i++) {
                bitstring_set_bit(&bit_string, (uint8_t)i, false);
            }
            bitstring_set_bit(&bit_string, OBJECT_DEVICE,        true);
            bitstring_set_bit(&bit_string, OBJECT_ANALOG_VALUE,  true);
            bitstring_set_bit(&bit_string, OBJECT_BINARY_VALUE,  true);
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_OBJECT_LIST:
            count = objectListCount();
            apdu_len = bacnet_array_encode(
                rpdata->object_instance, rpdata->array_index,
                objectListElementEncode, count, apdu, apdu_max);
            if (apdu_len == BACNET_STATUS_ABORT) {
                rpdata->error_code = ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
            } else if (apdu_len == BACNET_STATUS_ERROR) {
                rpdata->error_class = ERROR_CLASS_PROPERTY;
                rpdata->error_code  = ERROR_CODE_INVALID_ARRAY_INDEX;
            }
            break;
        case PROP_MAX_APDU_LENGTH_ACCEPTED:
            apdu_len = encode_application_unsigned(&apdu[0], MAX_APDU);
            break;
        case PROP_SEGMENTATION_SUPPORTED:
            apdu_len = encode_application_enumerated(&apdu[0], SEGMENTATION_NONE);
            break;
        case PROP_APDU_TIMEOUT:
            apdu_len = encode_application_unsigned(&apdu[0], 60000);
            break;
        case PROP_NUMBER_OF_APDU_RETRIES:
            apdu_len = encode_application_unsigned(&apdu[0], 0);
            break;
        case PROP_DEVICE_ADDRESS_BINDING:
            break;
        case PROP_DATABASE_REVISION:
            apdu_len = encode_application_unsigned(&apdu[0], 0);
            break;
        case PROP_MAX_INFO_FRAMES:
            apdu_len = encode_application_unsigned(&apdu[0], dlmstp_max_info_frames());
            break;
        case PROP_MAX_MASTER:
            apdu_len = encode_application_unsigned(&apdu[0], dlmstp_max_master());
            break;
        case (BACNET_PROPERTY_ID)9600:
            apdu_len = encode_application_unsigned(&apdu[0], RS485_Get_Baud_Rate());
            break;
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code  = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = -1;
            break;
    }
    if ((apdu_len >= 0) && (rpdata->object_property != PROP_OBJECT_LIST) &&
        (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code  = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }
    return apdu_len;
}

bool BACnetDevice::writeProperty(BACNET_WRITE_PROPERTY_DATA *wp_data)
{
    bool status = false;
    int len = 0;
    BACNET_APPLICATION_DATA_VALUE value = { 0 };
    size_t name_length;

    if (!validObjectInstanceNumber(wp_data->object_instance)) {
        wp_data->error_class = ERROR_CLASS_OBJECT;
        wp_data->error_code  = ERROR_CODE_UNKNOWN_OBJECT;
        return false;
    }
    len = bacapp_decode_application_data(
        wp_data->application_data, wp_data->application_data_len, &value);
    if (len < 0) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code  = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }
    if ((wp_data->object_property != PROP_OBJECT_LIST) &&
        (wp_data->array_index != BACNET_ARRAY_ALL)) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code  = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
    switch (wp_data->object_property) {
        case PROP_OBJECT_NAME:
            if (value.tag == BACNET_APPLICATION_TAG_CHARACTER_STRING) {
                name_length = characterstring_length(&value.type.Character_String);
                if (name_length == 0) {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code  = ERROR_CODE_VALUE_OUT_OF_RANGE;
                } else {
                    status = nvdata_name_write(NV_EEPROM_DEVICE_NAME,
                        &value.type.Character_String,
                        &wp_data->error_class, &wp_data->error_code);
                }
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code  = ERROR_CODE_INVALID_DATA_TYPE;
            }
            break;
        case PROP_DESCRIPTION:
            if (value.tag == BACNET_APPLICATION_TAG_CHARACTER_STRING) {
                name_length = characterstring_length(&value.type.Character_String);
                if (name_length == 0) {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code  = ERROR_CODE_VALUE_OUT_OF_RANGE;
                } else {
                    status = nvdata_name_write(NV_EEPROM_DEVICE_DESCRIPTION,
                        &value.type.Character_String,
                        &wp_data->error_class, &wp_data->error_code);
                }
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code  = ERROR_CODE_INVALID_DATA_TYPE;
            }
            break;
        case PROP_LOCATION:
            if (value.tag == BACNET_APPLICATION_TAG_CHARACTER_STRING) {
                name_length = characterstring_length(&value.type.Character_String);
                if (name_length == 0) {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code  = ERROR_CODE_VALUE_OUT_OF_RANGE;
                } else {
                    status = nvdata_name_write(NV_EEPROM_DEVICE_LOCATION,
                        &value.type.Character_String,
                        &wp_data->error_class, &wp_data->error_code);
                }
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code  = ERROR_CODE_INVALID_DATA_TYPE;
            }
            break;
        case PROP_OBJECT_IDENTIFIER:
        case PROP_MAX_INFO_FRAMES:
        case PROP_MAX_MASTER:
        case PROP_NUMBER_OF_APDU_RETRIES:
        case PROP_APDU_TIMEOUT:
        case PROP_VENDOR_IDENTIFIER:
        case PROP_SYSTEM_STATUS:
        case PROP_MODEL_NAME:
        case PROP_VENDOR_NAME:
        case PROP_FIRMWARE_REVISION:
        case PROP_APPLICATION_SOFTWARE_VERSION:
        case PROP_LOCAL_TIME:
        case PROP_UTC_OFFSET:
        case PROP_LOCAL_DATE:
        case PROP_DAYLIGHT_SAVINGS_STATUS:
        case PROP_PROTOCOL_VERSION:
        case PROP_PROTOCOL_REVISION:
        case PROP_PROTOCOL_SERVICES_SUPPORTED:
        case PROP_PROTOCOL_OBJECT_TYPES_SUPPORTED:
        case PROP_OBJECT_LIST:
        case PROP_MAX_APDU_LENGTH_ACCEPTED:
        case PROP_SEGMENTATION_SUPPORTED:
        case PROP_DEVICE_ADDRESS_BINDING:
        case PROP_DATABASE_REVISION:
        case PROP_ACTIVE_COV_SUBSCRIPTIONS:
        case 9600:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code  = ERROR_CODE_WRITE_ACCESS_DENIED;
            break;
        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code  = ERROR_CODE_UNKNOWN_PROPERTY;
            break;
    }
    return status;
}

/* =========================================================================
   APDU methods â€" mirrors apdu.c
   ========================================================================= */

bool BACnetDevice::apduServiceSupported(BACNET_SERVICES_SUPPORTED service_supported)
{
    bool status = false;
    if (service_supported == SERVICE_SUPPORTED_READ_PROPERTY) status = true;
    if (service_supported == SERVICE_SUPPORTED_WHO_IS)        status = true;
#ifdef WRITE_PROPERTY
    if (service_supported == SERVICE_SUPPORTED_WRITE_PROPERTY) status = true;
#endif
    return status;
}

static uint16_t apdu_decode_confirmed_request(
    uint8_t *apdu, uint16_t apdu_len,
    BACNET_CONFIRMED_SERVICE_DATA *service_data,
    uint8_t *service_choice,
    uint8_t **service_request, uint16_t *service_request_len)
{
    uint16_t len = 0;
    service_data->segmented_message           = (apdu[0] & BIT(3)) ? true : false;
    service_data->more_follows                = (apdu[0] & BIT(2)) ? true : false;
    service_data->segmented_response_accepted = (apdu[0] & BIT(1)) ? true : false;
    service_data->max_segs  = decode_max_segs(apdu[1]);
    service_data->max_resp  = decode_max_apdu(apdu[1]);
    service_data->invoke_id = apdu[2];
    service_data->priority  = BACnetDevice_NetworkPriority;
    len = 3;
    if (service_data->segmented_message) {
        service_data->sequence_number         = apdu[len++];
        service_data->proposed_window_number  = apdu[len++];
    }
    *service_choice      = apdu[len++];
    *service_request     = &apdu[len];
    *service_request_len = apdu_len - len;
    return len;
}

void BACnetDevice::apduHandler(
    BACNET_ADDRESS *src, uint8_t *apdu, uint16_t apdu_len)
{
    BACNET_CONFIRMED_SERVICE_DATA service_data = { 0 };
    uint8_t  service_choice = 0;
    uint8_t *service_request = NULL;
    uint16_t service_request_len = 0;
    uint16_t len = 0;

    if (!apdu) return;

    switch (apdu[0] & 0xF0) {
        case PDU_TYPE_CONFIRMED_SERVICE_REQUEST:
            len = apdu_decode_confirmed_request(
                &apdu[0], apdu_len, &service_data,
                &service_choice, &service_request, &service_request_len);
            if (len == 0) {
                DTFCLN(F("[apdu] confirmed req decode failed - len=0"));
                break;
            }
            DTFC(F("[apdu] confirmed service="));
            DTFCLN(service_choice);
            if (service_data.segmented_message) {
                DTFCLN(F("[apdu] WARN segmented - sending abort"));
            }
            if (service_choice == SERVICE_CONFIRMED_READ_PROPERTY) {
                DTFCLN(F("[apdu] -> ReadProperty"));
                handlerReadProperty(service_request, service_request_len,
                                    src, &service_data);
            }
#ifdef WRITE_PROPERTY
            else if (service_choice == SERVICE_CONFIRMED_WRITE_PROPERTY) {
                DTFCLN(F("[apdu] -> WriteProperty"));
                handlerWriteProperty(service_request, service_request_len,
                                     src, &service_data);
            }
#endif
            else {
                DTFC(F("[apdu] WARN unrecognized confirmed service="));
                DTFCLN(service_choice);
                handler_unrecognized_service(service_request, service_request_len,
                                             src, &service_data);
            }
            (void)len;
            break;
        case PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST:
            service_choice      = apdu[1];
            service_request     = &apdu[2];
            service_request_len = apdu_len - 2;
            DTFC(F("[apdu] unconfirmed service="));
            DTFCLN(service_choice);
            if (service_choice == SERVICE_UNCONFIRMED_WHO_IS) {
                DTFCLN(F("[apdu] -> WhoIs"));
                handlerWhoIs(service_request, service_request_len, src);
            }
            break;
        case PDU_TYPE_SIMPLE_ACK:
            DTFCLN(F("[apdu] rx SimpleAck (ignored)"));
            break;
        case PDU_TYPE_ERROR:
            DTFCLN(F("[apdu] rx Error (ignored)"));
            break;
        case PDU_TYPE_REJECT:
            DTFCLN(F("[apdu] rx Reject (ignored)"));
            break;
        case PDU_TYPE_ABORT:
            DTFCLN(F("[apdu] rx Abort (ignored)"));
            break;
        default:
            DTFC(F("[apdu] WARN unknown PDU type=0x"));
            DTFC2(apdu[0] & 0xF0, HEX);
            DTFCLN(F(""));
            break;
    }
}

/* =========================================================================
   ReadProperty handler â€" mirrors h_rp.c
   ========================================================================= */

int BACnetDevice::encodePropertyAPDU(BACNET_READ_PROPERTY_DATA *rpdata)
{
    int apdu_len = -1;

    switch (rpdata->object_type) {
        case OBJECT_DEVICE:
            if (rpdata->object_instance == BACNET_MAX_INSTANCE) {
                rpdata->object_instance = objectInstanceNumber();
            }
            if (validObjectInstanceNumber(rpdata->object_instance)) {
                apdu_len = readProperty(rpdata);
            }
            break;
        case OBJECT_ANALOG_VALUE:
            if (Analog_Value_Valid_Instance(rpdata->object_instance)) {
                apdu_len = Analog_Value_Read_Property(rpdata);
            }
            break;
        case OBJECT_BINARY_VALUE:
            if (Binary_Value_Valid_Instance(rpdata->object_instance)) {
                apdu_len = Binary_Value_Read_Property(rpdata);
            }
            break;
        default:
            rpdata->error_class = ERROR_CLASS_OBJECT;
            rpdata->error_code  = ERROR_CODE_UNKNOWN_OBJECT;
            break;
    }
    return apdu_len;
}

void BACnetDevice::handlerReadProperty(
    uint8_t *service_request, uint16_t service_len,
    BACNET_ADDRESS *src, BACNET_CONFIRMED_SERVICE_DATA *service_data)
{
    BACNET_READ_PROPERTY_DATA data;
    int len = 0, ack_len = 0, property_len = 0, pdu_len = 0, ack_end_len = 0;
    BACNET_NPDU_DATA npdu_data;
    BACNET_ADDRESS my_address;

    datalink_get_my_address(&my_address);
    npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len = npdu_encode_pdu(&Handler_Transmit_Buffer[0], src,
                               &my_address, &npdu_data);

    if (service_data->segmented_message) {
        DTFCLN(F("[rp] WARN segmented - abort SEGMENTATION_NOT_SUPPORTED"));
        len = abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                                service_data->invoke_id,
                                ABORT_REASON_SEGMENTATION_NOT_SUPPORTED, true);
        goto RP_ABORT;
    }
    len = rp_decode_service_request(service_request, service_len, &data);
    if (len < 0) {
        DTFCLN(F("[rp] ERROR decode failed - abort OTHER"));
        len = abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                                service_data->invoke_id, ABORT_REASON_OTHER, true);
        goto RP_ABORT;
    }
    DTFC(F("[rp] objType="));
    DTFC(data.object_type);
    DTFC(F("  inst="));
    DTFC(data.object_instance);
    DTFC(F("  prop="));
    DTFCLN(data.object_property);
    ack_len     = rp_ack_encode_apdu_init(&Handler_Transmit_Buffer[pdu_len],
                                          service_data->invoke_id, &data);
    ack_end_len = rp_ack_encode_apdu_object_property_end(NULL);
    data.application_data     = &Handler_Transmit_Buffer[pdu_len + ack_len];
    data.application_data_len = sizeof(Handler_Transmit_Buffer) -
                                 (pdu_len + ack_len + ack_end_len);
    data.error_class = ERROR_CLASS_OBJECT;
    data.error_code  = ERROR_CODE_UNKNOWN_OBJECT;
    property_len = encodePropertyAPDU(&data);
    if (property_len >= 0) {
        DTFC(F("[rp] OK  property_len="));
        DTFCLN(property_len);
        len = rp_ack_encode_apdu_object_property_end(
            &Handler_Transmit_Buffer[pdu_len + property_len + ack_len]);
        len += ack_len + property_len;
    } else if (property_len == BACNET_STATUS_ABORT) {
        DTFCLN(F("[rp] ERROR ABORT - segmentation not supported"));
        len = abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                                service_data->invoke_id,
                                ABORT_REASON_SEGMENTATION_NOT_SUPPORTED, true);
    } else {
        DTFC(F("[rp] ERROR class="));
        DTFC(data.error_class);
        DTFC(F("  code="));
        DTFCLN(data.error_code);
        len = bacerror_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                                   service_data->invoke_id,
                                   SERVICE_CONFIRMED_READ_PROPERTY,
                                   data.error_class, data.error_code);
    }
RP_ABORT:
    pdu_len += len;
    {
        int sent = datalink_send_pdu(src, &npdu_data, &Handler_Transmit_Buffer[0], pdu_len);
        if (sent == 0) {
            DTFCLN(F("[rp] WARN dlmstp_send_pdu dropped (TransmitPacketPending)"));
        }
    }
}

/* =========================================================================
   WriteProperty handler â€" mirrors h_wp.c
   ========================================================================= */

void BACnetDevice::handlerWriteProperty(
    uint8_t *service_request, uint16_t service_len,
    BACNET_ADDRESS *src, BACNET_CONFIRMED_SERVICE_DATA *service_data)
{
    int len = 0, pdu_len = 0;
    BACNET_NPDU_DATA npdu_data;
    BACNET_ADDRESS my_address;
    bool write_ok = false;
#if defined(BACNET_DEBUG_TRAFFIC)
    unsigned long t0 = micros();
#endif

    len = wp_decode_service_request(service_request, service_len, &_wpData);
    datalink_get_my_address(&my_address);
    npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len = npdu_encode_pdu(&Handler_Transmit_Buffer[0], src,
                               &my_address, &npdu_data);

    if (len <= 0) {
        DTFCLN(F("[wp] ERROR decode failed - abort OTHER"));
        len = abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                                service_data->invoke_id, ABORT_REASON_OTHER, true);
    } else if (service_data->segmented_message) {
        DTFCLN(F("[wp] WARN segmented - abort SEGMENTATION_NOT_SUPPORTED"));
        len = abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                                service_data->invoke_id,
                                ABORT_REASON_SEGMENTATION_NOT_SUPPORTED, true);
    } else {
        DTFC(F("[wp] objType="));
        DTFC(_wpData.object_type);
        DTFC(F("  inst="));
        DTFC(_wpData.object_instance);
        DTFC(F("  prop="));
        DTFC(_wpData.object_property);
        DTFC(F("  data_len="));
        DTFCLN(_wpData.application_data_len);
        _wpData.error_class = ERROR_CLASS_OBJECT;
        _wpData.error_code  = ERROR_CODE_UNKNOWN_OBJECT;
        switch (_wpData.object_type) {
            case OBJECT_DEVICE:
                write_ok = writeProperty(&_wpData);
                break;
            case OBJECT_ANALOG_VALUE:
                write_ok = Analog_Value_Write_Property(&_wpData);
                break;
            case OBJECT_BINARY_VALUE:
                write_ok = Binary_Value_Write_Property(&_wpData);
                break;
            default:
                DTFC(F("[wp] ERROR unknown object type="));
                DTFCLN(_wpData.object_type);
                break;
        }
        if (write_ok) {
#if defined(BACNET_DEBUG_TRAFFIC)
            DTFC(F("[wp] OK  SimpleAck  elapsed="));
            DTFC(micros() - t0);
            DTFCLN(F("us"));
#endif
            len = encode_simple_ack(&Handler_Transmit_Buffer[pdu_len],
                                    service_data->invoke_id,
                                    SERVICE_CONFIRMED_WRITE_PROPERTY);
        } else {
            DTFC(F("[wp] ERROR class="));
            DTFC(_wpData.error_class);
            DTFC(F("  code="));
            DTFCLN(_wpData.error_code);
            len = bacerror_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                                       service_data->invoke_id,
                                       SERVICE_CONFIRMED_WRITE_PROPERTY,
                                       _wpData.error_class, _wpData.error_code);
        }
    }
    pdu_len += len;
    {
        int sent = datalink_send_pdu(src, &npdu_data, &Handler_Transmit_Buffer[0], pdu_len);
        if (sent == 0) {
            DTFCLN(F("[wp] WARN dlmstp_send_pdu dropped (TransmitPacketPending)!"));
        } else {
            DTFC(F("[wp] sent pdu_len="));
            DTFCLN(pdu_len);
        }
    }
}

/* =========================================================================
   WhoIs handler â€" mirrors h_whois.c
   ========================================================================= */

void BACnetDevice::handlerWhoIs(
    uint8_t *service_request, uint16_t service_len, BACNET_ADDRESS *src)
{
    int32_t low_limit = 0, high_limit = 0;
    (void)src;

    int len = whois_decode_service_request(service_request, service_len,
                                           &low_limit, &high_limit);
    if (len == 0) {
        DTFCLN(F("[whois] broadcast - setting Send_I_Am_Flag"));
        Send_I_Am_Flag = true;
    } else if (len != BACNET_STATUS_ERROR) {
        int32_t target = (int32_t)objectInstanceNumber();
        DTFC(F("[whois] range="));
        DTFC(low_limit);
        DTFC(F(".."));
        DTFC(high_limit);
        DTFC(F("  device="));
        DTFCLN(target);
        if ((target >= low_limit) && (target <= high_limit)) {
            DTFCLN(F("[whois] match - setting Send_I_Am_Flag"));
            Send_I_Am_Flag = true;
        } else {
            DTFCLN(F("[whois] no match - Send_I_Am_Flag unchanged"));
        }
    } else {
        DTFCLN(F("[whois] ERROR decode failed"));
    }
}

/* =========================================================================
   extern "C" wrappers â€" satisfy device.h, apdu.h, services.h C signatures
   required by the upstream bacnet/ stack (dlmstp.c, npdu.c, tsm.c â€¦)
   ========================================================================= */
extern "C" {

/* device.h */
uint32_t Device_Object_Instance_Number(void)
{
    return BACnetDevice::objectInstanceNumber();
}

bool Device_Set_Object_Instance_Number(uint32_t object_id)
{
    return BACnetDevice::setObjectInstanceNumber(object_id);
}

bool Device_Valid_Object_Instance_Number(uint32_t object_id)
{
    return BACnetDevice::validObjectInstanceNumber(object_id);
}

uint16_t Device_Vendor_Identifier(void)
{
    return BACnetDevice::vendorIdentifier();
}

unsigned Device_Object_List_Count(void)
{
    return BACnetDevice::objectListCount();
}

bool Device_Object_List_Identifier(
    uint32_t array_index, BACNET_OBJECT_TYPE *object_type, uint32_t *instance)
{
    return BACnetDevice::objectListIdentifier(array_index, object_type, instance);
}

int Device_Object_List_Element_Encode(
    uint32_t object_instance, BACNET_ARRAY_INDEX array_index, uint8_t *apdu)
{
    return BACnetDevice::objectListElementEncode(object_instance, array_index, apdu);
}

int Device_Read_Property(BACNET_READ_PROPERTY_DATA *rpdata)
{
    return BACnetDevice::readProperty(rpdata);
}

bool Device_Write_Property(BACNET_WRITE_PROPERTY_DATA *wp_data)
{
    return BACnetDevice::writeProperty(wp_data);
}

bool apdu_service_supported(BACNET_SERVICES_SUPPORTED service_supported)
{
    return BACnetDevice::apduServiceSupported(service_supported);
}

void apdu_handler(BACNET_ADDRESS *src, uint8_t *apdu, uint16_t apdu_len)
{
    BACnetDevice::apduHandler(src, apdu, apdu_len);
}

/* services.h handler functions */
void handler_read_property(
    uint8_t *service_request, uint16_t service_len,
    BACNET_ADDRESS *src, BACNET_CONFIRMED_SERVICE_DATA *service_data)
{
    BACnetDevice::handlerReadProperty(service_request, service_len,
                                      src, service_data);
}

void handler_write_property(
    uint8_t *service_request, uint16_t service_len,
    BACNET_ADDRESS *src, BACNET_CONFIRMED_SERVICE_DATA *service_data)
{
    BACnetDevice::handlerWriteProperty(service_request, service_len,
                                       src, service_data);
}

void handler_who_is(
    uint8_t *service_request, uint16_t service_len, BACNET_ADDRESS *src)
{
    BACnetDevice::handlerWhoIs(service_request, service_len, src);
}

} /* extern "C" */
