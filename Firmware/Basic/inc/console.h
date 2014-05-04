/*
 * console.h
 *
 *  Created on: 04 мая 2014 г.
 *      Author: r.leonov
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_


#include "cmd_uart.h"
#include "application.h"



void Console_Send_Info(uint16_t ID, PayloadString_t *Ptr);

void Console_SetTime_Ack(int32_t NewCycDiff);

void Console_GetTime_Ack();

#endif /* CONSOLE_H_ */
