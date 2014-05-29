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

enum LongState_t {lsNone, lsSetType, lsPeleng};
enum PillState_t {piNone, piGood, piBad};

class Indication_t {
private:
    LedRGB_t Led;
    LongState_t LongState;
    PillState_t PillState;
    // Dose indication
    inline int32_t IDoseUmvos();
    inline int32_t IDoseDetectorMobile();
//    inline
public:
    Thread *PThd;
    VirtualTimer TmrClick;
    void Init();
    void Reset();
    // Commands
    void PillGood() { PillState = piGood; }
    void PillBad()  { PillState = piBad; }
    void HealthRenew();
    void PelengReceived();
    void PelengLost();
    void LustraBadID() {}
    // Inner use
    void ITask();
};

extern Indication_t Indication;


#endif /* INDICATION_H_ */
