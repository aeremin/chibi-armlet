/*
 * sequences.h
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#ifndef COLORS_SOUNDS_H_
#define COLORS_SOUNDS_H_

#include "peripheral.h"

/*
 * ckNormal => after this, goto next chunk
 * ckStop   => after this, stop and off
 * ckRepeat => after this, goto begin
 */

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

#if 1 // ============================ LED blink ================================
struct BlinkBeep_t {
    Color_t Color1;
    uint16_t Time1_ms;
    Color_t Color2;
    uint16_t Time2_ms;
    const BeepChunk_t *PBeep;
};
// Timings
#define T_SETTYPE_BLINK_MS  999 // Time to shine with device' color when type set
#define T_PELENG_BLINK_MS   3600

#if 1 // ==== Device type colors ====
const Color_t DeviceColor[] = {
        clDimGreen, // dtNothing
        clGreen,    // dtUmvos
        clBlue,     // dtLustraClean
        clGreen,    // dtLustraWeak
        clYellow,   // dtLustraStrong
        clMagenta,  // dtLustraLethal
        clCyan,     // dtDetectorMobile
        clCyan,     // dtDetectorFixed
        clBlue,     // dtEmpMech
        clRed,      // dtEmpGrenade
        clWhite,    // dtPelengator
        clDimBlue,  // dtPillFlasher
};
#endif

// Pill
const BlinkBeep_t BBPill[] = {
        {clBlack,  0, clBlack, 0,  nullptr},     // No Pill
        {clBlue, 702, clBlack, 504, BeepPillOk},  // Pill Good
        {clRed,  702, clBlack, 504, BeepPillBad}, // Pill Bad
};

// Battery

#if 1 // ==== Health states ====
const BlinkBeep_t BBHealth[] = {
        {clGreen,  36, clBlack, 3006, nullptr},   // hsGreen
        {clYellow, 36, clBlack, 3006, nullptr},   // hsYellow
        {clRed,    36, clBlack, 1008, nullptr},   // hsRedSlow
        {clRed,    36, clBlack, 54,   BeepShort}, // hsRedFast
        {clRed,    36, clRed,   9999, BeepDeath}, // hsDeath
};
#endif // health

#endif // Colors


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




#endif /* COLORS_SOUNDS_H_ */
