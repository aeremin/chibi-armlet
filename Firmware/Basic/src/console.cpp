/*
 * console.cpp
 *
 *  Created on: 04 мая 2014 г.
 *      Author: r.leonov
 */




#include "console.h"

void Console_Send_Info(uint16_t ID, PayloadString_t *Ptr) {
    Uart.Printf("#Node %u %u %u %d %u %u %u\r\n",
            ID,
            Ptr->Hops,
            Ptr->Timestamp,
            Ptr->TimeDiff,
            Ptr->Location,
            Ptr->Reason,
            Ptr->Emotion);
}

void Console_SetTime_Ack(int32_t NewCycDiff) {
    Uart.Printf("#Ack %X\r\n", (uint16_t)NewCycDiff);
}

void Console_GetMeshInfo_Ack() {
    Uart.Printf("#Ack %u %u\r\n", MAX_ABONENTS, CYCLE_TIME);
}
