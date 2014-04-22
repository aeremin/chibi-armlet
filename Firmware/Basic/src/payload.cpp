/*
 * payload.cpp
 *
 *  Created on: 15 апр. 2014 г.
 *      Author: r.leonov
 */


#include "payload.h"

Payload_t Payload;


uint8_t Payload_t::WriteInfo(uint16_t ID, int8_t RSSI, uint32_t TimeStampValue, PayloadString_t *Ptr) {
    uint8_t Rslt = FAILURE;
    Ptr->Hops += 1;
    Ptr->Timestamp = TimeStampValue;
    InfoBuf[ID] = *Ptr;
//    Uart.Printf("Pld: %u, %u\r", ID, Ptr->Hops);
    return Rslt;
}

uint8_t Payload_t::PrintNextInfo() {
    do {
        PStr++;
        if(PStr >= InfoBuf + INFO_BUF_SIZE) {
            PStr = InfoBuf;
            return LAST;
        }
    } while(PStr->Hops == 0);
    Uart.Printf("%u,%u,%u,%u,%u,%u,%u\r",
            (uint16_t)(PStr - InfoBuf),
            PStr->Hops,
            PStr->Timestamp,
            PStr->TimeDiff,
            PStr->Location,
            PStr->Reason,
            PStr->Emotion);
    return OK;
}

PayloadString_t* Payload_t::GetNextInfo(uint16_t *P) {
    do {
        PNext++;
        if(PNext >= InfoBuf + INFO_BUF_SIZE) {
            PNext = InfoBuf;
            *P = SELF_MESH_ID;
            return (&SelfInfo); // Self Info
        }
    } while(PNext->Hops == 0);
    *P = (uint16_t)(PNext - InfoBuf);
    return PNext;
}

void Payload_t::CorrectionTimeStamp(uint32_t CorrValue) {
    Uart.Printf("Correct to %u\r", CorrValue);
//    for(uint16_t i=0; i<INFO_BUF_SIZE; i++) {
//        if(InfoBuf[i].Hops != 0) {
//            InfoBuf[i].Timestamp -= CorrValue;
//            InfoBuf[i].TimeDiff -= CorrValue;
//        }
//    }
}
