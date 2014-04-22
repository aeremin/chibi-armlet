/*
 * radio_lvl1.h
 *
 *  Created on: Nov 17, 2013
 *      Author: kreyl
 */

#ifndef RADIO_LVL1_H_
#define RADIO_LVL1_H_

#include "ch.h"
#include "rlvl1_defins.h"
#include "kl_lib_L15x.h"
#include "cmd_uart.h"
#include "cc1101.h"
#include "payload.h"

class rLevel1_t {
private:
    rPkt_t PktRx, PktTx;
    Thread *PThread, *PAppThd;
    int32_t NaturalDmg, RadioDmg;

#ifdef MESH
    uint32_t RxTmt, Time;
    VirtualTimer MeshRxVT;
#endif

//    inline void GetDmgOutOfPkt() {
//        int32_t prc = RSSI_DB2PERCENT(PktRx.RSSI);
////      Uart.Printf("%u\r", PktRx.ID);
//        if(prc >= PktRx.MinLvl) {   // Only if signal level is enough
//            if((PktRx.DmgMax == 0) and (PktRx.DmgMin == 0)) NaturalDmg = 0; // "Clean zone" emanator
//            else {  // Ordinal emanator
//                int32_t EmDmg = 0;
//                if(prc >= PktRx.MaxLvl) EmDmg = PktRx.DmgMax;
//                else {
//                    int32_t DifDmg = PktRx.DmgMax - PktRx.DmgMin;
//                    int32_t DifLvl = PktRx.MaxLvl - PktRx.MinLvl;
//                    EmDmg = (prc * DifDmg + PktRx.DmgMax * DifLvl - PktRx.MaxLvl * DifDmg) / DifLvl;
//                    if(EmDmg < 0) EmDmg = 0;
//                }
//                RadioDmg += EmDmg;
//            }
//        }
//    }
    void IterateEmanators() {
        NaturalDmg = 1;
        RadioDmg = 0;
        // Iterate slow emanators
        for(uint8_t i=0; i<SLOW_EMANATOR_CNT; i++) {
            CC.SetChannel(CHANNEL_ZERO + i);
//            if(CC.ReceiveSync(27, &PktRx) == OK) {
//                GetDmgOutOfPkt();
//              if(RadioDmg != 0) Uart.Printf("%d; %d\r", prc, RadioDmg);
//            } // if ok
        } // for
        // Sleep until asked
        CC.Sleep();
        chSysLock();
        Damage = NaturalDmg + RadioDmg;
        chSysUnlock();
    }

    void TimeAgeKeeper() {
        if(PktTx.SelfID != PktTx.TimeOwnerID) PktTx.TimeAge++;
        if(PktTx.TimeAge > TIME_AGE_THRESHOLD) { ResetTimeAge(PktTx.SelfID); }
    }

public:
    Thread *PrThd;
    uint32_t Damage;
    void Init(uint16_t ASelfID);
    void SendEvt(eventmask_t mask) { if(PrThd != nullptr) chEvtSignal(PrThd, mask);}
    // Inner use
    void ITask();

#ifdef MESH
    void ResetTimeAge(uint8_t ID)     { PktTx.TimeAge = 0; PktTx.TimeOwnerID = ID; }
    uint8_t GetTimeAge()              { return PktTx.TimeAge;     }
    uint8_t GetTimeOwner()            { return PktTx.TimeOwnerID; }
    void SetTimeOwner(uint16_t ID)    { PktTx.TimeOwnerID = ID; }
    void PutCycle(uint32_t NewV)      { PktTx.CycleN = NewV;    }
    bool IMeshRx;
#endif

    void FillPayload(uint16_t AbbID, PayloadString_t *PayloadData);
};

extern rLevel1_t Radio;

#endif /* RADIO_LVL1_H_ */
