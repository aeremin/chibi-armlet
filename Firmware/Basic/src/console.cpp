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
    Uart.Printf("#%X,%d\r", RPL_SET_TIME, NewCycDiff);
}

void Console_GetTime_Ack() {
    Uart.Printf("#%X,%u\r", RPL_GET_MESH_INFO, CYCLE_TIME);
}
