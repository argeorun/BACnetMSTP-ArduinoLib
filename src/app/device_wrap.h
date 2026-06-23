/**
 * Copyright (c) 2025-2026 George Arun
 *
 * This file is part of the BACnet MSTP Arduino library wrapper layer.
 * Licensed under the MIT License. See LICENSE in the project root.
 */

/**
 * @file device_wrap.h
 * @brief Companion declaration header for device.c — C++ wrapper use only.
 *
 * Declares exactly the 9 public functions that device.c implements.
 * Do NOT include src/bacnet/basic/object/device.h here — that header
 * declares callbacks and data structures device.c does not implement,
 * which would produce link errors from C++.
 *
 * This header is included ONLY by src/wrapper/BACnetDevice.h inside
 * an extern "C" {} block.  No existing .c file includes this header.
 */

#ifndef APP_DEVICE_WRAP_H
#define APP_DEVICE_WRAP_H

#include "../compile_config.h"  /* must be first */
#include <stdint.h>
#include <stdbool.h>

/* Minimal upstream type headers — each has its own include guard */
#include "../bacnet/bacdef.h"   /* BACNET_OBJECT_TYPE, BACNET_ARRAY_INDEX  */
#include "../bacnet/rp.h"       /* BACNET_READ_PROPERTY_DATA               */
#include "../bacnet/wp.h"       /* BACNET_WRITE_PROPERTY_DATA              */

#ifdef __cplusplus
extern "C" {
#endif

/* --- All 9 public functions implemented in device.c --- */

uint32_t  Device_Object_Instance_Number       (void);
bool      Device_Set_Object_Instance_Number   (uint32_t object_id);
bool      Device_Valid_Object_Instance_Number (uint32_t object_id);
uint16_t  Device_Vendor_Identifier            (void);
unsigned  Device_Object_List_Count            (void);
bool      Device_Object_List_Identifier       (uint32_t array_index,
                                               BACNET_OBJECT_TYPE *object_type,
                                               uint32_t           *instance);
int       Device_Object_List_Element_Encode   (uint32_t           object_instance,
                                               BACNET_ARRAY_INDEX array_index,
                                               uint8_t           *apdu);
int       Device_Read_Property                (BACNET_READ_PROPERTY_DATA  *rpdata);
bool      Device_Write_Property               (BACNET_WRITE_PROPERTY_DATA *wp_data);

#ifdef __cplusplus
}
#endif

#endif /* APP_DEVICE_WRAP_H */
