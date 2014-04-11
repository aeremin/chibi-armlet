/*
 * SensorTable.h
 *
 *  Created on: 02 февр. 2014 г.
 *      Author: Kreyl
 */

#ifndef SENSORTABLE_H_
#define SENSORTABLE_H_

#include "kl_lib_L15x.h"
#include "mesh_lvl.h"

struct Row_t {
    uint16_t    ID;
    uint8_t     Level;
//    uint8_t     Hops;
//    uint32_t    Timestamp;
//    uint32_t    TimeDiff;
//    uint8_t     Location;
//    uint8_t     Reason;
//    uint8_t     Emotion;
} __attribute__ ((__packed__));

#define MAX_ROW_CNT  99

struct Table_t {
    uint32_t Size;
    Row_t Row[MAX_ROW_CNT];
}__attribute__ ((__packed__));

class SnsTable_t {
private:
    Table_t ITbl[2], *PCurrTbl;
    void ISwitchTableI();
    Thread *IPThd;
public:
    void RegisterAppThd(Thread *PThd) { IPThd = PThd; }
    SnsTable_t() : PCurrTbl(&ITbl[0]), IPThd(nullptr), PTable(&ITbl[1]) {}
    Table_t *PTable;
    void PutSnsInfo(uint8_t ID, uint32_t Level);
    void SendEvtReady();
};

extern SnsTable_t SnsTable;

#endif /* SENSORTABLE_H_ */
