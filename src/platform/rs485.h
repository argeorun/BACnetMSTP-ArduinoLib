/**
 * @brief The module handles API for the RS-485 port
 * @author Steve Karg <skarg@users.sourceforge.net>
 * @author George Arun (Arduino HardwareSerial port, 2025-2026)
 * @date 2007
 * @copyright SPDX-License-Identifier: MIT
 *
 * Step 4 note: implementation moved from rs485.cpp to BACnetRS485.cpp.
 * This header is kept unchanged so dlmstp.c continues to link without
 * modification. All symbols are provided via extern "C" wrappers in
 * BACnetRS485.cpp.
 */
#ifndef RS485_H
#define RS485_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

unsigned long RS485_Timer_Silence(void);
void          RS485_Timer_Silence_Reset(void);
void          RS485_Initialize(void);
void          RS485_Transmitter_Enable(bool enable);
void          RS485_Send_Data(const uint8_t *buffer, uint16_t nbytes);
bool          RS485_ReceiveError(void);
bool          RS485_DataAvailable(uint8_t *data);
void          RS485_Turnaround_Delay(void);
uint32_t      RS485_Get_Baud_Rate(void);
bool          RS485_Set_Baud_Rate(uint32_t baud);
uint32_t      RS485_Baud_Rate_From_Kilo(uint8_t baud_k);
void          RS485_LED_Timers(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* RS485_H */
