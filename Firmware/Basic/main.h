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
#include "Sequences.h"

#define APP_NAME            "LettersOfEast_RX"
#define APP_VERSION         _TIMENOW_

// ==== Constants and default values ====
#define ID_MIN                  1
#define ID_MAX                  10

// Timings
#define RX_CHECK_PERIOD_MS      2700
#define INDICATION_TIME_MS      18000

class App_t {
private:
    Thread *PThread;
//    uint32_t SavedCnt;
//    const LedRGBChunk_t *lsqSaved = nullptr;
    const LedWsChunk_t  *wsqSaved = wsqNone;
public:
    VirtualTimer TmrCheck, TmrOff;
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
