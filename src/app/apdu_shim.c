/**
 * @file apdu_shim.c
 * @brief C-linkage definitions for apdu_network_priority functions
 * @author George Arun
 * @date 2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * apdu.h has no extern "C" guards.  When included from a .cpp file the
 * declarations get C++ linkage, so defining apdu_network_priority* in
 * BACnetDevice.cpp causes a linkage conflict.  h_npdu.c (a C file) also
 * calls apdu_network_priority_set and expects a C-linkage symbol.
 *
 * Solution: define the two functions here in a plain C file where they
 * naturally get C linkage.  The shared state is held in
 * BACnetDevice_NetworkPriority, which BACnetDevice.cpp exports as
 * extern "C" so this file can access it.
 *
 */

#include "../bacnet/apdu.h"

/* Defined as extern "C" in BACnetDevice.cpp */
extern uint8_t BACnetDevice_NetworkPriority;

uint8_t apdu_network_priority(void)
{
    return BACnetDevice_NetworkPriority;
}

void apdu_network_priority_set(uint8_t pri)
{
    BACnetDevice_NetworkPriority = pri & 0x03;
}
