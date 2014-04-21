/*
 * RxTable.h
 *
 *  Created on: 02 февр. 2014 г.
 *      Author: Kreyl
 */

#ifndef SENSORTABLE_H_
#define SENSORTABLE_H_

#include "kl_lib_L15x.h"
#include "cmd_uart.h"
#include "ch.h"
#include "evt_mask.h"
#include "rlvl1_defins.h"
#include "peripheral.h"
#include "sequences.h"

struct Row_t {
    uint8_t ID;
    uint8_t Type;
    int32_t Rssi;
};

#define MAX_ROW_CNT     99
struct Table_t {
    uint32_t Size;
    Row_t Row[MAX_ROW_CNT];
};

class RxTable_t {
private:
    Table_t ITbl[2], *PWriteTbl;
//    Thread *IPThd;
public:
//    void RegisterAppThd(Thread *PThd) { IPThd = PThd; }
    Table_t *PTable = &ITbl[1];

    void PutInfo(uint8_t ID, uint8_t Type, int8_t Rssi) {
        //    Uart.Printf("ID=%u,RSSI=%u\r", ID, Level);
        if(PWriteTbl->Size >= MAX_ROW_CNT) return;
        for(uint32_t i=0; i<PWriteTbl->Size; i++) {
            if(PWriteTbl->Row[i].ID == ID) {
                PWriteTbl->Row[i].Rssi = MAX(PWriteTbl->Row[i].Rssi, Rssi);
                return;
            }
        }
        PWriteTbl->Row[PWriteTbl->Size].ID = ID;
        PWriteTbl->Row[PWriteTbl->Size].Rssi = Rssi;
        PWriteTbl->Row[PWriteTbl->Size].Type = Type;
        PWriteTbl->Size++;
    }

//    void SendEvtReady() {
//        chSysLock();
//        ISwitchTableI();
//        if(IPThd != nullptr) chEvtSignalI(IPThd, EVTMSK_RX_TABLE_READY);
//        chSysUnlock();
//    }

    void dBm2Percent() {
        for(uint32_t i=0; i < PTable->Size; i++) {
            int32_t Rssi = PTable->Row[i].Rssi;
            if(Rssi < -100) Rssi = -100;
            else if(Rssi > -15) Rssi = -15;
            Rssi += 100;    // 0...85
            PTable->Row[i].Rssi  = dBm2Percent1000Tbl[Rssi];
        }
    }

    void Print() {
        if(PTable->Size == 0) return;
//        Uart.Printf("ID; Type; Rssi;\r");
        for(uint32_t i=0; i < PTable->Size; i++) {
            if(PTable->Row[i].Rssi > 500) {
//                Uart.Printf("%u; %u; %d\r", PTable->Row[i].ID, PTable->Row[i].Type, PTable->Row[i].Rssi);
//                Beeper.Beep(BeepShort);
            }
        }
    }

    void SwitchTable() {
        chSysLock();
        if(PWriteTbl == &ITbl[0]) {
            PWriteTbl = &ITbl[1];
            PTable    = &ITbl[0];
        }
        else {
            PWriteTbl = &ITbl[0];
            PTable    = &ITbl[1];
        }
        PWriteTbl->Size = 0; // Reset table
        chSysUnlock();
    }
};

#endif /* SENSORTABLE_H_ */
