/**
 * @file BACnetMSTP.h
 * @brief Single-include umbrella header for the BACnetMSTP-ArduinoLib library
 * @author George Arun
 * @date 2026
 * @copyright SPDX-License-Identifier: MIT
 *
 * Single include for all BACnetMSTP-ArduinoLib classes:
 * BACnetStorage, BACnetHardware, BACnetRS485, BACnetAV,
 * BACnetBV, BACnetDevice (and its AVProxy / BVProxy).
 */

#pragma once

#include "platform/BACnetStorage.h"
#include "platform/BACnetHardware.h"
#include "platform/BACnetRS485.h"
#include "app/BACnetAV.h"
#include "app/BACnetBV.h"
#include "app/BACnetDevice.h"
