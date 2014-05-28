/*
 * indication.h
 *
 *  Created on: 28 мая 2014 г.
 *      Author: Kreyl
 */

#ifndef INDICATION_H_
#define INDICATION_H_

#include "peripheral.h"
#include "evt_mask.h"

#if 1 // ==== Timings ====
#define TM_CLICK_MS          18      // for Detector
#endif

class Indication_t {
private:
    LedRGB_t Led;
    Beeper_t Beeper;
public:
    Thread *PThd;
    VirtualTimer TmrClick;
    void Init();
    void Reset();
    // Commands
    void PillGood();
    void PillBad();
    void HealthRenew();
    void PelengReceived();
    void PelengLost();
    // Inner use
    void ITask();
};

extern Indication_t Indication;


#endif /* INDICATION_H_ */
