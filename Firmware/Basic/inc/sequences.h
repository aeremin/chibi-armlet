/*
 * sequences.h
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#ifndef SEQUENCES_H_
#define SEQUENCES_H_

#include "peripheral.h"

#if 1 // ============================ LED RGB ==================================
const LedChunk_t ShortGreen[] = {
        {clGreen, 18, ckNormal},
        {clBlack, 990, ckLast},
};

const LedChunk_t LedRedFast[] = {
        {clRed,   36, ckNormal},
        {clBlack, 36, ckLast},
};
const LedChunk_t LedRedSlow[] = {
        {clRed,   36, ckNormal},
        {clBlack, 999, ckLast},
};
const LedChunk_t LedYellow[] = {
        {clYellow, 36, ckNormal},
        {clBlack,  999, ckLast},
};
const LedChunk_t LedGreen[] = {
        {clGreen, 36, ckNormal},
        {clBlack, 999, ckLast},
};

#endif

#if 1 // ============================= Beep ====================================
/* Every sequence is an array of BeepCmd_t:
 struct BeepCmd_t {
    uint8_t VolumePercent;
    uint32_t Time_ms;
    uint16_t Freq_Hz;
  };
*/
#define BEEP_END    {-1, 0, 0}
#define BEEP_REPEAT {-2, 0, 0}

const BeepChunk_t ShortBeep[] = {
        {9, 45, 2000},
        BEEP_END
};

const BeepChunk_t BeepBeep[] = {
        {9, 54, 2000},
        {0, 54},
        {9, 54, 2000},
        BEEP_END
};

const BeepChunk_t LongBeep[] = {
        {100, 4000, 2000},
        BEEP_END
};

const BeepChunk_t BeepDeath[] = {
        {9, 2000, 2000},
        {0, 10000},
        BEEP_REPEAT
};
const BeepChunk_t BeepRedFast[] = {
        {9, 54, 2000},
        {0, 54},
        BEEP_REPEAT
};
const BeepChunk_t BeepBeepLoud[] = {
        {9, 54, 2000},
        {0, 54},
        {9, 54, 2000},
        BEEP_END
};

#endif

#endif /* SEQUENCES_H_ */
