/*
 * beeper.h
 *
 *  Created on: 22 марта 2015 г.
 *      Author: Kreyl
 */

#ifndef KL_LIB_BEEPER_H_
#define KL_LIB_BEEPER_H_

#include "ChunkTypes.h"
#include "kl_lib.h"

#define BEEP_TOP_VALUE   22

class Beeper_t : public BaseSequencer_t<BeepChunk_t> {
private:
    PinOutputPWM_t<BEEP_TOP_VALUE, invNotInverted, omPushPull> IPin;
    void ISwitchOff() { IPin.Set(0); }
    SequencerLoopTask_t ISetup() {
        IPin.SetFrequencyHz(IPCurrentChunk->Freq_Hz);
        IPin.Set(IPCurrentChunk->Volume);
        IPCurrentChunk++;   // Always goto next
        return sltProceed;  // Always proceed
    }
public:
    Beeper_t() : BaseSequencer_t(), IPin(GPIOB, 3, TIM2, 2) {}
    void Init() { IPin.Init(); }
    void Beep(uint32_t Freq_Hz, uint8_t Volume) {
        IPin.SetFrequencyHz(Freq_Hz);
        IPin.Set(Volume);
    }
    void Off() { IPin.Set(0); }
};


#endif /* KL_LIB_BEEPER_H_ */
