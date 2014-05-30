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

enum PillState_t {piNone, piGood, piBad};

class Indication_t {
private:
    LedRGB_t Led;
    PillState_t PillState;
//    void WaitEvent(uint32_t t_ms);
public:
    Thread *PThd;
    void Init();
    // Commands
    void PillGood() { PillState = piGood; chEvtSignal(PThd, EVTMSK_PILL_CHECK); }
    void PillBad()  { PillState = piBad;  chEvtSignal(PThd, EVTMSK_PILL_CHECK); }
    void PelengReceived() { }
    void ProcessTypeChange();
    void LustraBadID() {}
    // Inner use
    inline void ITask();
    inline int32_t ITaskUmvos();
    inline int32_t ITaskDetectorMobile();
};

extern Indication_t Indication;


#endif /* INDICATION_H_ */
