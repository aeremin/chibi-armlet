/*
 * sequences.h
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#ifndef SEQUENCES_H_
#define SEQUENCES_H_

#include "peripheral.h"

/*
 * ckNormal => after this, goto next chunk
 * ckStop   => after this, stop and off
 * ckRepeat => after this, goto begin
 */

#if 1 // ============================ LED RGB ==================================
const LedChunk_t LedRedFast[] = {
        {clRed,   36, ckNormal},
        {clBlack, 36, ckRepeat},
};
const LedChunk_t LedRedSlow[] = {
        {clRed,   36, ckNormal},
        {clBlack, 999, ckRepeat},
};
const LedChunk_t LedYellow[] = {
        {clYellow, 36, ckNormal},
        {clBlack,  999, ckRepeat},
};
const LedChunk_t LedGreen[] = {
        {clGreen, 36, ckNormal},
        {clBlack, 999, ckRepeat},
};

#endif

#if 1 // ============================= Beep ====================================
/* Every sequence is an array of BeepCmd_t:
 struct BeepChunk_t {
    uint8_t Volume;   // 0 means silence, 10 means top
    uint16_t Freq_Hz;
    uint16_t Time_ms;
    ChunkKind_t ChunkKind;
  };
*/
#define BEEP_VOLUME     1   // set to 10 in production, and to 1 when someone sleeps near

const BeepChunk_t BeepBeep[] = {
        {BEEP_VOLUME, 1975, 54, ckNormal},
        {0, 0, 54, ckNormal},
        {BEEP_VOLUME, 1975, 54, ckStop},
};

// Pill
const BeepChunk_t BeepPillOk[] = {
        {BEEP_VOLUME, 1975, 180, ckNormal},
        {BEEP_VOLUME, 2489, 180, ckNormal},
        {BEEP_VOLUME, 2960, 180, ckStop},
};

const BeepChunk_t BeepPillBad[] = {
        {BEEP_VOLUME, 2794, 180, ckNormal},
        {BEEP_VOLUME, 2349, 180, ckNormal},
        {BEEP_VOLUME, 1975, 180, ckStop},
};


// Health states
const BeepChunk_t BeepDeath[] = {
        {BEEP_VOLUME, 2000, 2000, ckNormal},
        {0, 0, 10000, ckRepeat},
};
const BeepChunk_t BeepRedFast[] = {
        {BEEP_VOLUME, 2000, 54, ckNormal},
        {0, 0, 54, ckRepeat},
};
#endif

#endif /* SEQUENCES_H_ */
