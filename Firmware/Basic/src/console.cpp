/*
 * console.cpp
 *
 *  Created on: 04 мая 2014 г.
 *      Author: r.leonov
 */




#include "console.h"

Console_t Console;

void Console_t::Send_Info(uint16_t ID, AlienInfo_t *Ptr) {
    Cnt++;
    if(Cnt == SEND_IN_COUNT) {
        Uart.Printf("#Node %u %u %u %d %u %u %u\r\n",
                ID,
                Ptr->Mesh.Hops,
                Ptr->Mesh.Timestamp,
                Ptr->Mesh.TimeDiff,
                Ptr->State.Location,
                Ptr->State.Reason,
                Ptr->State.Emotion);
        Cnt = 0;
    }
}

void Console_t::SetTime_Ack(int32_t NewCycDiff) {
    Uart.Printf("#MeshCycle %d\r\n", NewCycDiff);
}

void Console_t::GetMeshInfo_Ack(uint32_t Rslt) {
    if(Rslt == OK) {
        Uart.Printf("#MeshInfo %u %u\r\n", MAX_ABONENTS, CYCLE_TIME);
    }
    else Uart.Ack(Rslt);
}
