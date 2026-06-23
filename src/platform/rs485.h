/**
 * @brief The module handles API for the RS-485 port
 * @author Steve Karg <skarg@users.sourceforge.net>
 * @author George Arun (Arduino HardwareSerial port, 2025-2026)
 * @date 2007
 * @copyright SPDX-License-Identifier: MIT
 */
#ifndef RS485_H
#define RS485_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

unsigned long RS485_Timer_Silence(void);

void RS485_Timer_Silence_Reset(void);

void RS485_Initialize(void);

void RS485_Transmitter_Enable(bool enable);

void RS485_Send_Data(
    const uint8_t *buffer, /* data to send */
    uint16_t nbytes); /* number of bytes of data */

bool RS485_ReceiveError(void);
bool RS485_DataAvailable(uint8_t *data);

void RS485_Turnaround_Delay(void);
uint32_t RS485_Get_Baud_Rate(void);
bool RS485_Set_Baud_Rate(uint32_t baud);
uint32_t RS485_Baud_Rate_From_Kilo(uint8_t baud_k);

void RS485_LED_Timers(void);

/* Runtime DE/RE pin override (default: compile-time constant in rs485.cpp) */
void RS485_Set_Enable_Pin(uint8_t pin);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* Runtime serial port override — C++ callers only (HardwareSerial is a C++ type). */
#ifdef __cplusplus
class HardwareSerial;
void RS485_Set_Serial_Port(HardwareSerial *port);
#endif

#endif
