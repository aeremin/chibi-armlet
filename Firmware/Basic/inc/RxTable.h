/*
 * RxTable.h
 *
 *  Created on: 02 февр. 2014 г.
 *      Author: Kreyl
 */

#ifndef SENSORTABLE_H_
#define SENSORTABLE_H_

#include "kl_lib_L15x.h"
#include "ch.h"
#include "evt_mask.h"

struct Row_t {
    uint8_t ID;
    uint8_t Type;
    int8_t Rssi;
};

#define MAX_ROW_CNT     99
struct Table_t {
    uint32_t Size;
    Row_t Row[MAX_ROW_CNT];
};

class RxTable_t {
private:
    Table_t ITbl[2], *PCurrTbl;
    Thread *IPThd;
    void ISwitchTableI() {
        if(PCurrTbl == &ITbl[0]) {
            PCurrTbl = &ITbl[1];
            PTable   = &ITbl[0];
        }
        else {
            PCurrTbl = &ITbl[0];
            PTable   = &ITbl[1];
        }
        PCurrTbl->Size = 0; // Reset table
    }
public:
    void RegisterAppThd(Thread *PThd) { IPThd = PThd; }
    RxTable_t() : PCurrTbl(&ITbl[0]), IPThd(nullptr), PTable(&ITbl[1]) {}
    Table_t *PTable;

    void PutInfo(uint8_t ID, uint8_t Type, int8_t Rssi) {
        //    Uart.Printf("ID=%u,RSSI=%u\r", ID, Level);
        if(PCurrTbl->Size >= MAX_ROW_CNT) return;
        for(uint32_t i=0; i<PCurrTbl->Size; i++) {
            if(PCurrTbl->Row[i].ID == ID) {
                PCurrTbl->Row[i].Rssi = MAX(PCurrTbl->Row[i].Rssi, Rssi);
                return;
            }
        }
        PCurrTbl->Row[PCurrTbl->Size].ID = ID;
        PCurrTbl->Row[PCurrTbl->Size].Rssi = Rssi;
        PCurrTbl->Row[PCurrTbl->Size].Type = Type;
        PCurrTbl->Size++;
    }

    void SendEvtReady() {
        chSysLock();
        ISwitchTableI();
        if(IPThd != nullptr) chEvtSignalI(IPThd, EVTMSK_RX_TABLE_READY);
        chSysUnlock();
    }
};

#endif /* SENSORTABLE_H_ */
