/*
 * application.h
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "kl_lib.h"
#include "uart.h"
#include "evt_mask.h"
#include "ChunkTypes.h"

#define VERSION_STRING  "Ragnarok v1.0"

#define RXBUF_CHECK_PERIOD_MS   3600
#define INDICATION_PERIOD_MS    1800

// ==== RX table ====
#define RX_TABLE_SZ     54
class RxTable_t {
private:
    uint32_t IBuf[RX_TABLE_SZ];
public:
    uint32_t Cnt;
    void AddID(uint32_t ID) {
        for(uint32_t i=0; i<Cnt; i++) {
            if(IBuf[i] == ID) return;   // do not add what exists
        }
        IBuf[Cnt] = ID;
        Cnt++;
    }
    void Clear() { Cnt = 0; }
};

class App_t {
private:
    Thread *PThread;
    uint8_t ISetID(int32_t NewID);
    RxTable_t RxTable;
    const BaseChunk_t *VibroChunk = nullptr;
public:
    uint32_t UID;
    VirtualTimer TmrCheck, TmrIndication;
    // Eternal methods
    void InitThread() { PThread = chThdSelf(); }
    void SignalEvt(eventmask_t Evt) {
        chSysLock();
        chEvtSignalI(PThread, Evt);
        chSysUnlock();
    }
    void SignalEvtI(eventmask_t Evt) { chEvtSignalI(PThread, Evt); }
    void OnUartCmd(Uart_t *PUart);
    // Inner use
    void ITask();
//    App_t(): PThread(nullptr), ID(ID_DEFAULT), Mode(mRxVibro) {}
};

extern App_t App;

#endif /* APPLICATION_H_ */
