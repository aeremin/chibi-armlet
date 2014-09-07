/*
 * sequences.h
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#ifndef SEQUENCES_H_
#define SEQUENCES_H_

#include "color.h"

#if 1 // ============================ LED RGB ==================================
// ==== Led Chunk ====
enum LedChunkSort_t {csSetColor, csWait, csJump, csEnd};
struct LedChunk_t {
    LedChunkSort_t ChunkSort;
    union {
        uint32_t SmoothVar;       // 0 means immediately. Up to several thounds
        uint32_t Time_ms;
        uint32_t ChunkToJumpTo;
    };
    Color_t Color;
};
#define LED_CHUNK_SZ   sizeof(LedChunk_t)

// Battery
const LedChunk_t LedBatteryDischarged[] = {
        {csSetColor, 0, clRed}, // Set color immediately
        {csWait, 99},
        {csSetColor, 0, clBlack},
        {csEnd}
};
//Tuman
const LedChunk_t LedTumanBeg[] = {
        {csSetColor, 180, clBlue},
        {csEnd}
};
const LedChunk_t LedTumanEnd[] = {
        {csSetColor, 180, clBlack},
        {csEnd}
};

// Someone Near
const LedChunk_t LedSomeoneIsNear[] = {
        {csSetColor, 720, clGreen},
        {csEnd}
};
const LedChunk_t LedNobodyHere[] = {
        {csSetColor, 720, clBlack},
        {csEnd}
};

// ID
const LedChunk_t LedGoodID[] = {
        {csSetColor, 0, clGreen},
        {csWait, 504},
        {csSetColor, 0, clBlack},
        {csEnd}
};

const LedChunk_t LedBadID[] = {
        {csSetColor, 0, clRed}, // Set color immediately
        {csWait, 504},
        {csSetColor, 0, clBlack},
        {csEnd}
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
enum ChunkSort_t {ckNormal, ckEnd, ckJump};
/*
 * ckNormal => after this, goto next chunk
 * ckStop   => after this, stop and off
 * ckRepeat => after this, goto begin
 */
struct BeepChunk_t {
    uint8_t Volume;   // 0 means silence, 10 means top
    uint16_t Freq_Hz;
    uint16_t Time_ms;
    ChunkSort_t ChunkSort;
};
#define BEEP_CHUNK_SZ   sizeof(BeepChunk_t)

#define BEEP_VOLUME     2   // set to 10 in production, and to 1 when someone sleeps near

const BeepChunk_t BeepBeep[] = {
        {BEEP_VOLUME, 1975, 54, ckNormal},
        {0, 0, 54, ckNormal},
        {BEEP_VOLUME, 1975, 54, ckEnd},
};

const BeepChunk_t BeepShort[] = {
        {BEEP_VOLUME, 1975, 54, ckEnd},
};

// Pill
const BeepChunk_t BeepPillOk[] = {
        {BEEP_VOLUME, 1975, 180, ckNormal},
        {BEEP_VOLUME, 2489, 180, ckNormal},
        {BEEP_VOLUME, 2960, 180, ckEnd},
};

const BeepChunk_t BeepPillBad[] = {
        {BEEP_VOLUME, 2794, 180, ckNormal},
        {BEEP_VOLUME, 2349, 180, ckNormal},
        {BEEP_VOLUME, 1975, 180, ckEnd},
};

// Health states
const BeepChunk_t BeepDeath[] = {
        {BEEP_VOLUME, 1975, 2000, ckNormal},
        {0, 0, 10000, ckJump},
};
const BeepChunk_t BeepRedFast[] = {
        {BEEP_VOLUME, 1975, 54, ckNormal},
        {0, 0, 54, ckJump},
};
#endif

#if VIBRO_ENABLED // =================== Vibro =================================
/* Every sequence is an array of VibroChunk_t:
struct VibroChunk_t {
    StateOnOff_t OnOff;
    uint16_t Time_ms;
    ChunkKind_t ChunkKind;
};
*/
const VibroChunk_t Brr[] = {
        {stOn,  180, ckStop},
};

const VibroChunk_t BrrBrr[] = {
        {stOn,  99, ckNormal},
        {stOff, 180, ckNormal},
        {stOn,  99, ckStop},
};


#define BRR_MS          54
#define BRR_LONG_MS     99
#define BRR_PAUSE_MS    54
const VibroChunk_t Brr1[] = {
        {stOn,  BRR_LONG_MS, ckStop},
};
const VibroChunk_t Brr2[] = {
        {stOn,  BRR_LONG_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckStop},
};
const VibroChunk_t Brr3[] = {
        {stOn,  BRR_LONG_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckStop},
};
const VibroChunk_t Brr4[] = {
        {stOn,  BRR_LONG_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckStop},
};
const VibroChunk_t Brr5[] = {
        {stOn,  BRR_LONG_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckStop},
};
const VibroChunk_t Brr6[] = {
        {stOn,  BRR_LONG_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckStop},
};
const VibroChunk_t Brr7[] = {
        {stOn,  BRR_LONG_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckStop},
};
const VibroChunk_t Brr8[] = {
        {stOn,  BRR_LONG_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckStop},
};
const VibroChunk_t Brr9[] = {
        {stOn,  BRR_LONG_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckStop},
};
const VibroChunk_t Brr10[] = {
        {stOn,  BRR_LONG_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckNormal}, {stOff, BRR_PAUSE_MS, ckNormal},
        {stOn,  BRR_MS, ckStop},
};



#endif

#endif /* SEQUENCES_H_ */
