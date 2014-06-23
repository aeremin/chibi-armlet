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


uint8_t Payload_t::WriteInfo(uint16_t ID, uint32_t TimeStampDiff, PayloadString_t *Ptr) {
    uint8_t Rslt = FAILURE;
    if(ID == App.ID) {
        Uart.Printf("Payload with my ID!\r\n");
        Rslt = OK;
    }
//    else if(Ptr->Hops > InfoBuf[ID].Hops) Rslt = OK;
    else if(Ptr->Timestamp < InfoBuf[ID].Timestamp) Rslt = OK;
    else {
        uint8_t Hops = Ptr->Hops;
        Hops += 1;

        Ptr->Hops = Hops;
        Ptr->Timestamp -= TimeStampDiff;
        Ptr->TimeDiff -= TimeStampDiff;
        InfoBuf[ID] = *Ptr;
//        Uart.Printf("From Payload -> ");
//        Console.Send_Info(ID, &InfoBuf[ID]);
    }
    return Rslt;
}

uint16_t Payload_t::GetNextInfoID() {
    do{
        PNext++;
        if(PNext >= (InfoBuf + INFO_BUF_SIZE)) PNext = InfoBuf; // Remove "-1"
    } while(PNext->Timestamp == 0);
    return (uint16_t)((PNext - InfoBuf));
}

void Payload_t::WriteMesh(uint32_t CurrSelfCycle, MeshPayload_t *Ptr) {
    PayloadString_t *p = &InfoBuf[Ptr->SelfID];
    p->Hops = 1;
    p->Timestamp = CurrSelfCycle;
    p->TimeDiff = (int32_t)(Ptr->CycleN - CurrSelfCycle);
    p->Location = Ptr->SelfLocation;
    p->Reason = Ptr->SelfReason;
    p->Emotion = Ptr->SelfEmotion;
//    Uart.Printf("From mesh -> ");
//    Console.Send_Info(Ptr->SelfID, &InfoBuf[Ptr->SelfID]);
}


void Payload_t::UpdateSelf() {
    InfoBuf[App.ID].Timestamp = Mesh.GetCycleN();
    Console.Send_Info(App.ID, &InfoBuf[App.ID]);
}
void Payload_t::CorrectionTimeStamp(uint32_t CorrValueMS) {
    uint32_t CorrValueCycle = CorrValueMS/CYCLE_TIME;
    if(CorrValueCycle == 0) return;
//    Uart.Printf("Correct to %u\r", CorrValueCycle);
    for(uint16_t i=0; i<INFO_BUF_SIZE; i++) { // TODO: compare new time in new cycle
        if(InfoBuf[i].Timestamp != 0) {
            InfoBuf[i].Timestamp -= CorrValueCycle;
            InfoBuf[i].TimeDiff -= CorrValueCycle;
        }
    }
}
