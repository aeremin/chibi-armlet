/*
 * console.cpp
 *
 *  Created on: 04 мая 2014 г.
 *      Author: r.leonov
 */




#include "console.h"

void Console_Send_Info(uint16_t ID, PayloadString_t *Ptr) {
    Uart.Printf("%u,%u,%u,%d,%u,%u,%u\r",
            ID,
            Ptr->Hops,
            Ptr->Timestamp,
            Ptr->TimeDiff,
            Ptr->Location,
            Ptr->Reason,
            Ptr->Emotion);
}

void Console_SetTime_Ack(int32_t NewCycDiff) {
    Uart.Printf("#%X,%d\r", CMD_SET_TIME_RPL, NewCycDiff);
}

void Console_GetTime_Ack() {
    Uart.Printf("#%X,%u\r", CMD_GET_CYCLE_TIME_RPL, CYCLE_TIME);
}
