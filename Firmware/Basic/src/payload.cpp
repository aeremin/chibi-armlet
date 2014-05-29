/*
 * payload.cpp
 *
 *  Created on: 15 апр. 2014 г.
 *      Author: r.leonov
 */

#include "application.h"
#include "payload.h"
#include "mesh_lvl.h"
#include "console.h"

Payload_t Payload;


uint8_t Payload_t::WriteInfo(uint16_t ID, uint32_t CurrSelfCycle, PayloadString_t *Ptr) {
//    PayloadString_t *pPS = &TmpPayload;
    uint8_t Rslt = FAILURE;
    if(ID == App.ID) Rslt = OK;
    else {
        int32_t TimeDiff = Ptr->Timestamp;
        uint8_t Hops = Ptr->Hops;
        Hops += 1;
        TimeDiff -= CurrSelfCycle;

        Ptr->Hops = Hops;
        Ptr->Timestamp = CurrSelfCycle;
        Ptr->TimeDiff = TimeDiff;
        InfoBuf[ID] = *Ptr;
        Console.Send_Info(ID, &InfoBuf[ID]);
    }
    return Rslt;
}

uint16_t Payload_t::GetNextInfoID() {
    do{
        PNext++;
        if(PNext > (InfoBuf + INFO_BUF_SIZE - 1)) PNext = InfoBuf;
    } while(PNext->Timestamp == 0);
    return (uint16_t)((PNext - InfoBuf));
}

void Payload_t::WritePayload(uint16_t IDv, uint8_t Hops, uint32_t TimeStampValue, uint8_t NewLocation, uint8_t NewReason, uint8_t NewEmotion) {
    PayloadString_t *p = &InfoBuf[IDv];
    p->Hops = Hops;
    p->Timestamp = TimeStampValue;
    p->Location = NewLocation;
    p->Reason = NewReason;
    p->Emotion = NewEmotion;
}


void Payload_t::UpdateSelf() {
    InfoBuf[App.ID].Timestamp = Mesh.GetCycleN();
    Console.Send_Info(App.ID, &InfoBuf[App.ID]);
}
void Payload_t::CorrectionTimeStamp(uint32_t CorrValueMS) {
    uint32_t CorrValueCycle = CorrValueMS/CYCLE_TIME;
//    Uart.Printf("Correct to %u\r", CorrValueCycle);
    for(uint16_t i=0; i<INFO_BUF_SIZE; i++) {
        if(InfoBuf[i].Hops != 0) {
            InfoBuf[i].Timestamp -= CorrValueCycle;
            InfoBuf[i].TimeDiff -= CorrValueCycle;
        }
    }
}
