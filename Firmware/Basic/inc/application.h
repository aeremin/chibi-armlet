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
#define CMD_PILL_STATE      0x30
#define CMD_PILL_WRITE      0x31
#define CMD_PILL_READ       0x32
#define CMD_DOSE_GET        0x60
#define CMD_DOSE_SET        0x61

#define RPL_ACK             0x90    // Acknowledge
#define RPL_PILL_STATE      0xC0
#define RPL_PILL_WRITE      0xC1
#define RPL_PILL_READ       0xC2
#define RPL_DOSE_GET        0xE0
#endif

#if 1 // ==== Timings ====
#define TM_DOSE_INCREASE_MS     999
#define TM_DOSE_SAVE_MS         2007
#define TM_PILL_CHECK_MS        504     // Check if pill connected every TM_PILL_CHECK
#endif

#if 1 // ==== Dose constants ====
#define DOSE_TOP            1800  // Death; top value
#define DOSE_RED_FAST       (DOSE_TOP-7)  // Near death
#define DOSE_RED_SLOW       1200  // Yellow if lower
#define DOSE_YELLOW         600  // Green if lower
#endif

#define INFINITY16          (uint16_t)0xFFFF
#define INFINITY32          (uint16_t)0xFFFFFFFF

#if 1 // ==== Pill IDs
#define PILL_ID_CURE        0x09
#endif

class App_t {
private:

public:
    uint16_t ID;
    void Init();
    // Inner use
    void PillHandler();
};

extern App_t App;

#endif /* APPLICATION_H_ */
