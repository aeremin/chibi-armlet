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

#if 1 // ============================ LED blink ================================
#if 1 // ==== Device type colors ====
// Table of colors depending on type
#define DEVICETYPE_BLINK_T_MS   999
const LedChunk_t TypeColorTbl[] = {
        {clDimGreen, DEVICETYPE_BLINK_T_MS, ckStop}, // dtNothing
        {clGreen,    DEVICETYPE_BLINK_T_MS, ckStop}, // dtUmvos
        {clBlue,     DEVICETYPE_BLINK_T_MS, ckStop}, // dtLustraClean
        {clGreen,    DEVICETYPE_BLINK_T_MS, ckStop}, // dtLustraWeak
        {clYellow,   DEVICETYPE_BLINK_T_MS, ckStop}, // dtLustraStrong
        {clMagenta,  DEVICETYPE_BLINK_T_MS, ckStop}, // dtLustraLethal
        {clCyan,     DEVICETYPE_BLINK_T_MS, ckStop}, // dtDetectorMobile
        {clCyan,     DEVICETYPE_BLINK_T_MS, ckStop}, // dtDetectorFixed
        {clBlue,     DEVICETYPE_BLINK_T_MS, ckStop}, // dtEmpMech
        {clRed,      DEVICETYPE_BLINK_T_MS, ckStop}, // dtEmpGrenade
        {clWhite,    DEVICETYPE_BLINK_T_MS, ckStop}, // dtPelengator
        {clDimBlue,  DEVICETYPE_BLINK_T_MS, ckStop}, // dtPillFlasher
};
#endif

// Pill
const LedChunk_t LedPillCureOk[] = { {clBlue,  540, ckStop}, };
const LedChunk_t LedPillBad[] =    { {clRed,  540, ckStop},  };

const LedChunk_t LedPillIdSet[]    = { {clCyan,  999, ckStop},   };
const LedChunk_t LedPillIdNotSet[] = { {clYellow,  999, ckStop}, };
const LedChunk_t LedPillSetupOk[]  = { {clGreen,  999, ckStop},  };

// Battery
const LedChunk_t LedBatteryDischarged[] = { {clRed,  180, ckStop}, };

// Bad ID
const LedChunk_t LedBadID[] = { {{99, 0, 0},  99, ckStop}, };

#if 1 // Health states
const LedChunk_t LedRedFast[] = {
        {clRed,   36, ckNormal},
        {clBlack, 36, ckRepeat},
};
const LedChunk_t LedRedSlow[] = {
        {clRed,   36, ckNormal},
        {clBlack, 1008, ckRepeat},
};
const LedChunk_t LedYellow[] = {
        {clYellow, 36, ckNormal},
        {clBlack,  3006, ckRepeat},
};
const LedChunk_t LedGreen[] = {
        {clGreen, 36, ckNormal},
        {clBlack, 3006, ckRepeat},
};
#endif // health

#endif // Colors

// Snd coeffs
#define DMG_SND_MAX     1000
#define DMG_SND_BCKGND  40
#define DMG_MAX         50      // Maximum radiation value
// Just for example
#define DMG_SND_MID     220
#define DMG_SND_HEAVY   700


#if 1 // ============================= Beep ====================================
/* Every sequence is an array of BeepCmd_t:
 struct BeepChunk_t {
    uint8_t Volume;   // 0 means silence, 10 means top
    uint16_t Freq_Hz;
    uint16_t Time_ms;
    ChunkKind_t ChunkKind;
  };
*/
#define BEEP_VOLUME     2   // set to 10 in production, and to 1 when someone sleeps near

const BeepChunk_t BeepBeep[] = {
        {BEEP_VOLUME, 1975, 54, ckNormal},
        {0, 0, 54, ckNormal},
        {BEEP_VOLUME, 1975, 54, ckStop},
};

const BeepChunk_t BeepShort[] = {
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
        {BEEP_VOLUME, 1975, 2000, ckNormal},
        {0, 0, 10000, ckRepeat},
};
const BeepChunk_t BeepRedFast[] = {
        {BEEP_VOLUME, 1975, 54, ckNormal},
        {0, 0, 54, ckRepeat},
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
