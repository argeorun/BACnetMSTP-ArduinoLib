/**
 * Copyright (c) 2025-2026 George Arun
 *
 * This file is part of the BACnet MSTP Arduino library wrapper layer.
 * Licensed under the MIT License. See LICENSE in the project root.
 */

/**
 * @file bv_wrap.h
 * @brief Companion declaration header for bv.c — C++ wrapper use only.
 *
 * Declares exactly the 12 public functions that bv.c implements.
 * Do NOT include src/bacnet/basic/object/bv.h here — that header carries
 * symbols bv.c does not implement and would cause link errors from C++.
 *
 * This header is included ONLY by src/wrapper/BACnetBinaryValue.h inside
 * an extern "C" {} block.  No existing .c file includes this header.
 */

#ifndef APP_BV_WRAP_H
#define APP_BV_WRAP_H

#include "../compile_config.h"  /* must be first */
#include <stdint.h>
#include <stdbool.h>

/* Minimal upstream type headers — each has its own include guard */
#include "../bacnet/bacdef.h"   /* BACNET_OBJECT_TYPE, BACNET_ARRAY_INDEX  */
#include "../bacnet/rp.h"       /* BACNET_READ_PROPERTY_DATA               */
#include "../bacnet/wp.h"       /* BACNET_WRITE_PROPERTY_DATA              */
#include "../bacnet/bacenum.h"  /* BACNET_BINARY_PV                        */

#ifdef __cplusplus
extern "C" {
#endif

/* --- All 12 public functions implemented in bv.c --- */

unsigned         Binary_Value_Instance_To_Index (uint32_t instance);
bool             Binary_Value_Valid_Instance    (uint32_t instance);
unsigned         Binary_Value_Count             (void);
uint32_t         Binary_Value_Index_To_Instance (unsigned index);
const char      *Binary_Value_Name_ASCII        (uint32_t instance);
BACNET_BINARY_PV Binary_Value_Present_Value     (uint32_t instance);
bool             Binary_Value_Present_Value_Set (uint32_t instance, BACNET_BINARY_PV value);
int              Binary_Value_Read_Property     (BACNET_READ_PROPERTY_DATA  *rpdata);
bool             Binary_Value_Write_Property    (BACNET_WRITE_PROPERTY_DATA *wp);
void             Binary_Value_Init              (void);
uint8_t          Binary_Value_Pin               (uint32_t instance);
bool             Binary_Value_Is_Output         (uint32_t instance);

#ifdef __cplusplus
}
#endif

#endif /* APP_BV_WRAP_H */
