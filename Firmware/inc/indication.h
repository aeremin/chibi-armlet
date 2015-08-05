/*
 * indication.h
 *
 *  Created on: 28 мая 2014 г.
 *      Author: Kreyl
 */

#ifndef INDICATION_H_
#define INDICATION_H_

#include "evt_mask.h"
#include "colors_sounds.h"
#include "led.h"

#if 1 // ==== Timings ====
#define TM_CLICK_MS          18      // for Detector
#endif

enum PillState_t {piNone, piGood, piBad};
enum BatteryState_t {bsGood, bsBad};

class Indication_t {
private:
    LedRGB_t Led;
    Beeper_t Beeper;
    PillState_t PillState;
    bool IPelengReceived;
    DeviceType_t MaxSignalLvlDevType;
    void DoBeepBlink(const BlinkBeep_t *Pbb);
    // Device-dependent tasks
    int32_t ITaskUmvos();
    int32_t ITaskDetectorMobile();
    int32_t ITaskDetectorFixed();
    int32_t ITaskGrenade();
    int32_t ITaskEmpMech();
    int32_t ITaskPelengator();
    int32_t ITaskPillFlasher();
public:
    Thread *PThd;
    BatteryState_t BatteryState;
    void Init();
    // Commands
    void PillGood() { PillState = piGood; chEvtSignal(PThd, EVTMSK_PILL_CHECK); }
    void PillBad()  { PillState = piBad;  chEvtSignal(PThd, EVTMSK_PILL_CHECK); }
    void PelengReceived() { IPelengReceived = true; }
//    void PelengLost()     { IPelengReceived = false; }
    void ProcessTypeChange();
    void AutodocCompleted() { chEvtSignal(PThd, EVTMSK_AUTODOC_COMPLETED); }
    void AutodocExhausted() { chEvtSignal(PThd, EVTMSK_AUTODOC_EXHAUSTED); }
    void LustraBadID()      { chEvtSignal(PThd, EVTMSK_LUSTRA_BAD_ID); }
    void PelengatorDevTypeReceived(DeviceType_t DevType) { MaxSignalLvlDevType = DevType; }
    void JustWakeup() { chEvtSignal(PThd, EVTMSK_JUST_WAKEUP); }
    // Inner use
    void ITask();
};

extern Indication_t Indication;


#endif /* INDICATION_H_ */
