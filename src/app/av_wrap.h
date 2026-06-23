/**
 * @file av_wrap.h
 * @brief Companion declaration header for av.c — C++ wrapper use only.
 *
 * Declares exactly the 13 public functions that av.c implements.
 * Do NOT include src/bacnet/basic/object/av.h here — that header declares
 * ANALOG_VALUE_DESCR, event callbacks and other symbols av.c does not
 * implement; referencing them from C++ would cause link errors.
 *
 * This header is included ONLY by src/wrapper/BACnetAnalogValue.h inside
 * an extern "C" {} block.  No existing .c file includes this header.
 */

#ifndef APP_AV_WRAP_H
#define APP_AV_WRAP_H

#include "../compile_config.h"  /* must be first */
#include <stdint.h>
#include <stdbool.h>

/* Minimal upstream type headers — each has its own include guard */
#include "../bacnet/bacdef.h"   /* BACNET_OBJECT_TYPE, BACNET_ARRAY_INDEX  */
#include "../bacnet/rp.h"       /* BACNET_READ_PROPERTY_DATA               */
#include "../bacnet/wp.h"       /* BACNET_WRITE_PROPERTY_DATA              */
#include "../bacnet/bacenum.h"  /* BACNET_ENGINEERING_UNITS                */

#ifdef __cplusplus
extern "C" {
#endif

/* --- All 13 public functions implemented in av.c --- */

bool                     Analog_Value_Valid_Instance    (uint32_t object_instance);
unsigned                 Analog_Value_Count             (void);
uint32_t                 Analog_Value_Index_To_Instance (unsigned index);
unsigned                 Analog_Value_Instance_To_Index (uint32_t object_instance);
bool                     Analog_Value_Name_Set          (uint32_t object_instance, const char *value);
const char              *Analog_Value_Name_ASCII        (uint32_t object_instance);
float                    Analog_Value_Present_Value     (uint32_t object_instance);
bool                     Analog_Value_Present_Value_Set (uint32_t object_instance, float value, uint8_t priority);
BACNET_ENGINEERING_UNITS Analog_Value_Units             (uint32_t object_instance);
bool                     Analog_Value_Units_Set         (uint32_t object_instance, BACNET_ENGINEERING_UNITS units);
int                      Analog_Value_Read_Property     (BACNET_READ_PROPERTY_DATA  *rpdata);
bool                     Analog_Value_Write_Property    (BACNET_WRITE_PROPERTY_DATA *wp_data);
void                     Analog_Value_Init              (void);

#ifdef __cplusplus
}
#endif

#endif /* APP_AV_WRAP_H */
