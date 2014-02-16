/*
 * application.h
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "kl_lib_L15x.h"

# if 1 // Uart Command Codes. See https://docs.google.com/document/d/1pGQf9CrQ016ObS0w7PhPLAy92MRPhdBriICflt1YGXA/edit
#define CMD_PING            0x01

#define RPL_ACK             0x90    // Acknowledge
#endif

#if 1 // ==== Timings ====
#define TM_CHECK_MS         999     // Check if pill connected every TM_PILL_CHECK
#endif

class App_t {
public:
    Thread *PThd;
    void Init();
    // Inner use
    void IPillHandler();
    void ITask();
};

extern App_t App;

#endif /* APPLICATION_H_ */
