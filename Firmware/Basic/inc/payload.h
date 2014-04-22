/*
 * payload.h
 *
 *  Created on: 15 апр. 2014 г.
 *      Author: r.leonov
 */

#ifndef PAYLOAD_H_
#define PAYLOAD_H_


#include "kl_lib_L15x.h"
#include "cmd_uart.h"
#include "mesh_params.h"

#define INFO_BUF_SIZE   MAX_ABONENTS-1


struct PayloadString_t {
    uint8_t Hops;
    uint32_t Timestamp;
    uint32_t TimeDiff;
    uint8_t Location;
    uint8_t Reason;
    uint8_t Emotion;
} __attribute__ ((__packed__));


struct Payload_t {
private:
    PayloadString_t InfoBuf[INFO_BUF_SIZE];
    PayloadString_t SelfInfo;
public:
    Payload_t(): PStr(InfoBuf),
                 PNext(InfoBuf)   {}
    PayloadString_t *PStr, *PNext;
    uint8_t WriteInfo(uint16_t ID, int8_t RSSI, uint32_t TimeStampValue, PayloadString_t *Ptr);
    void FillSelfPayload(uint32_t TimeStampValue, uint8_t NewLocation, uint8_t NewReason, uint8_t NewEmotion) {
        SelfInfo.Timestamp = TimeStampValue;
        SelfInfo.Location = NewLocation;
        SelfInfo.Reason   = NewReason;
        SelfInfo.Emotion  = NewEmotion;
    }
    PayloadString_t* GetInfoByID(uint16_t ID) { return (PayloadString_t*)&InfoBuf[ID]; }
    PayloadString_t* GetNextInfo(uint16_t *P);
    uint8_t PrintNextInfo();
    void CorrectionTimeStamp(uint32_t CorrValue);
};

extern Payload_t Payload;

#endif /* PAYLOAD_H_ */
