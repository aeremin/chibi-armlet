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
#include "application.h"

#define INFO_BUF_SIZE   MAX_ABONENTS-1


struct Payload_t {
private:
    PayloadString_t InfoBuf[INFO_BUF_SIZE];
public:
    Payload_t(): PStr(InfoBuf),
                 PNext(InfoBuf)   {}
    PayloadString_t *PStr, *PNext;
    uint8_t WriteInfo(uint16_t ID, uint32_t CurrSelfCycle, PayloadString_t *Ptr);
    void WritePayload(uint16_t IDv, uint32_t TimeStampValue, uint8_t NewLocation, uint8_t NewReason, uint8_t NewEmotion);
    void NewLocation(uint32_t NewLoc)       { InfoBuf[App.ID].Location = NewLoc;    }
    void NewReason(uint32_t NewReason)      { InfoBuf[App.ID].Reason = NewReason;   }
    void NewEmotion(uint32_t NewEmotion)    { InfoBuf[App.ID].Emotion = NewEmotion; }
    PayloadString_t* GetInfoByID(uint16_t ID) { return (PayloadString_t*)&InfoBuf[ID]; }
    uint16_t GetNextInfoID();
    uint8_t PrintNextInfo();
    void UpdateSelf();
    void CorrectionTimeStamp(uint32_t CorrValue);
};

extern Payload_t Payload;

#endif /* PAYLOAD_H_ */
