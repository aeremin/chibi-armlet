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

#define APP_NAME            "FirstDaysOfEmpire TX"

// ==== Constants and default values ====
#define EE_DEVICE_ID_ADDR   0x04
#define MIN_ID              1
#define MAX_ID              30


// Timings
#define RX_CHECK_PERIOD_MS      2700
#define INDICATION_TIME_MS      9000

class App_t {
private:
    Thread *PThread;
public:
    uint8_t ID;
    uint32_t TimeS = 0;
    uint8_t ISetID(int32_t NewID);
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
