/*
 * payload.h
 *
 *  Created on: 15 апр. 2014 г.
 *      Author: r.leonov
 */

#ifndef PAYLOAD_H_
#define PAYLOAD_H_


#include "kl_lib_L15x.h"
#include "mesh_params.h"
#include "rlvl1_defins.h"


class PayloadString_t {
    uint32_t Timestamp;
    uint32_t TimeDiff;
    union {
        uint32_t Info;
        struct {
            uint8_t Hops;
            uint8_t Location;
            uint8_t Reason;
            uint8_t Emotion;
        };
    };
} __attribute__ ((__packed__));


struct Payload_t {
private:
    PayloadString_t InfoBuf[MAX_ABONENTS-1];
public:
    Payload_t(): PStr(InfoBuf) {}
    PayloadString_t* PStr;
    uint8_t WriteInfo(uint16_t ID, rPkt_t *P);
};

extern Payload_t Payload;

#endif /* PAYLOAD_H_ */
