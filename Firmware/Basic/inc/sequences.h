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

const BeepChunk_t ShortBeep[] = {
        {100, 45, 2000},
        BEEP_END
};

const BeepChunk_t BeepBeep[] = {
        {9, 54, 2000},
        {0, 54},
        {9, 54, 2000},
        BEEP_END
};

const BeepChunk_t LongBeep[] = {
        {100, 10000, 1000},
        BEEP_END
};
#endif

#endif /* SEQUENCES_H_ */
