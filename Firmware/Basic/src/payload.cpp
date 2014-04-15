/*
 * payload.cpp
 *
 *  Created on: 15 апр. 2014 г.
 *      Author: r.leonov
 */


#include "payload.h"

Payload_t Payload;


uint8_t Payload_t::WriteInfo(uint16_t ID, int8_t RSSI, PayloadString_t *Ptr) {
    uint8_t Rslt = FAILURE;
    Uart.Printf("Pld: [%u] %d dBm {%u,%u,%u,%u,%u,%u}\r", ID, RSSI, Ptr->Hops, Ptr->Timestamp, Ptr->TimeDiff, Ptr->Location, Ptr->Reason, Ptr->Emotion);
    return Rslt;
}
