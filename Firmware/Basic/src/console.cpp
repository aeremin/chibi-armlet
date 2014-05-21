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
//    Uart.Printf("#%X,%X\r", RPL_SET_TIME, (uint16_t)NewCycDiff);
}

void Console_GetMeshInfo_Ack() {
//    Uart.Printf("#%X,%u,%u\r", RPL_GET_MESH_INFO, MAX_ABONENTS, CYCLE_TIME);
}
