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
    uint8_t Rslt = FAILURE;
//    if(InfoBuf[ID].Timestamp < CurrentTimeStamp) {
    if(ID == App.ID) return Rslt;
    Ptr->Hops += 1;
//    Uart.Printf("%d, %d\r\n", (Ptr->Timestamp), CurrSelfCycle);
    Ptr->TimeDiff =  (Ptr->Timestamp) - CurrSelfCycle;
    Ptr->Timestamp = CurrSelfCycle;
    InfoBuf[ID] = *Ptr;
//    }
//    Console_Send_Info(ID, &InfoBuf[ID]);
    return Rslt;
}

uint8_t Payload_t::PrintNextInfo() {
    do {
        PStr++;
        if(PStr == InfoBuf + App.ID)  break;
        else if(PStr >= InfoBuf + INFO_BUF_SIZE) PStr = InfoBuf;
    } while (PStr->Hops == 0);
    Console_Send_Info((uint16_t)(PStr - InfoBuf), PStr);
    return OK;
}

uint16_t Payload_t::GetNextInfoID() {
    do {
        PNext++;
        if(PNext >= InfoBuf + INFO_BUF_SIZE) {
            PNext = InfoBuf;
            return (uint16_t)App.ID; // Self Info
        }
    } while(PNext->Hops == 0);
    return (uint16_t)(PNext - InfoBuf);
}

void Payload_t::WritePayload(uint16_t IDv, uint32_t TimeStampValue, uint8_t NewLocation, uint8_t NewReason, uint8_t NewEmotion) {
    PayloadString_t *p = &InfoBuf[IDv];
    p->Timestamp = TimeStampValue;
    p->Location = NewLocation;
    p->Reason = NewReason;
    p->Emotion = NewEmotion;
}


void Payload_t::UpdateSelf() {
    InfoBuf[App.ID].Timestamp = Mesh.GetCycleN();
//    Console_Send_Info(App.ID, &InfoBuf[App.ID]);
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
