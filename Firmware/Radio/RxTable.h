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

struct Row_t {
    uint8_t ID;
    uint8_t LvlMin;
    uint8_t LvlMax;
    uint8_t DmgMin;
    uint8_t DmgMax;
    int32_t Rssi;
};

#define ROW_CNT     LUSTRA_CNT
struct Table_t {
    uint32_t Size;
    Row_t Row[ROW_CNT];
};

class RxTable_t {
private:
    Table_t ITbl[2], *PWriteTbl;
public:
    Table_t *PTable = &ITbl[1];

    void PutPkt(rPkt_t *PPkt, int8_t Rssi) {
        //    Uart.Printf("ID=%u,RSSI=%u\r", ID, Level);
        if(PWriteTbl->Size >= ROW_CNT) return;
        uint32_t i = 0;
        while(i < PWriteTbl->Size) {
            if(PWriteTbl->Row[i].ID == PPkt->ID) { // If same ID found, compare new and saved RSSI.
                if(Rssi >= PWriteTbl->Row[i].Rssi) break; // Replace with new data if RSSI is bigger.
                else return;    // Otherwise just get out
            }
            i++;
        }
        PWriteTbl->Row[i].ID = PPkt->ID;
        PWriteTbl->Row[i].Rssi = Rssi;
        PWriteTbl->Row[i].LvlMin = PPkt->LvlMin;
        PWriteTbl->Row[i].LvlMax = PPkt->LvlMax;
        PWriteTbl->Row[i].DmgMin = PPkt->DmgMin;
        PWriteTbl->Row[i].DmgMax = PPkt->DmgMax;
        if(i >= PWriteTbl->Size) PWriteTbl->Size++; // Increase size if new row added
    }

    void SwitchTableI() {
        if(PWriteTbl == &ITbl[0]) {
            PWriteTbl = &ITbl[1];
            PTable    = &ITbl[0];
        }
        else {
            PWriteTbl = &ITbl[0];
            PTable    = &ITbl[1];
        }
        PWriteTbl->Size = 0; // Reset table
    }

    void dBm2PercentAll() {
        for(uint32_t i=0; i < PTable->Size; i++)
            PTable->Row[i].Rssi = dBm2Percent(PTable->Row[i].Rssi);
    }

    void Print() {
        if(PTable->Size == 0) return;
        Uart.Printf("ID; LvlMin; LvlMax; DmgMin; DmgMax; Rssi;\r");
        for(uint32_t i=0; i < PTable->Size; i++) {

//            if(PTable->Row[i].Rssi > 500) {
                Uart.Printf("%u; %u; %u; %u; %u; %d\r",
                        PTable->Row[i].ID,
                        PTable->Row[i].LvlMin,
                        PTable->Row[i].LvlMax,
                        PTable->Row[i].DmgMin,
                        PTable->Row[i].DmgMax,
                        PTable->Row[i].Rssi);
//                Beeper.Beep(BeepShort);
//            }
        }
    }


};

#endif /* SENSORTABLE_H_ */
