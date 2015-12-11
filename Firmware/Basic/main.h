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

#define VERSION_STRING  "Lustra " __TIMENOW__

class App_t {
private:
    Thread *PThread;
    VirtualTimer ITmrRadioTimeout;
    uint8_t ISetID(int32_t NewID);
public:
    int32_t ID;
    VirtualTimer TmrSecond;
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
};

extern App_t App;

#endif /* APPLICATION_H_ */
