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
#define DMG_MAX         1800      // Maximum radiation value: strong lustra
// Just for example
#define DMG_SND_MID     220
#define DMG_SND_HEAVY   700


#if 1 // ============================= Beep ====================================
#if 1 // ==== Notes ====
#define La_2    880

#define Do_3    1047
#define Do_D_3  1109
#define Re_3    1175
#define Re_D_3  1245
#define Mi_3    1319
#define Fa_3    1397
#define Fa_D_3  1480
#define Sol_3   1568
#define Sol_D_3 1661
#define La_3    1720
#define Si_B_3  1865
#define Si_3    1976

#define Do_4    2093
#define Do_D_4  2217
#define Re_4    2349
#define Re_D_4  2489
#define Mi_4    2637
#define Fa_4    2794
#define Fa_D_4  2960
#define Sol_4   3136
#define Sol_D_4 3332
#define La_4    3440
#define Si_B_4  3729
#define Si_4    3951

// Length
#define OneSixteenth    90
#define OneEighth       (OneSixteenth * 2)
#define OneFourth       (OneSixteenth * 4)
#define OneHalfth       (OneSixteenth * 8)
#define OneWhole        (OneSixteenth * 16)

#endif

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
        {BEEP_VOLUME, Si_3, 54, ckNormal},
        {0, 0, 54, ckNormal},
        {BEEP_VOLUME, Si_3, 54, ckStop},
};

const BeepChunk_t BeepShort[] = {
        {BEEP_VOLUME, Si_3, 54, ckStop},
};

// Pill
const BeepChunk_t BeepPillOk[] = {
        {BEEP_VOLUME, Si_3,   180, ckNormal},
        {BEEP_VOLUME, Re_D_4, 180, ckNormal},
        {BEEP_VOLUME, Fa_D_4, 180, ckStop},
};

const BeepChunk_t BeepPillBad[] = {
        {BEEP_VOLUME, Fa_4, 180, ckNormal},
        {BEEP_VOLUME, Re_4, 180, ckNormal},
        {BEEP_VOLUME, Si_3, 180, ckStop},
};

// Autodoc
const BeepChunk_t BeepAutodocCompleted[] = {
        {BEEP_VOLUME, Fa_3, OneHalfth, ckNormal}, {0, 0, 18, ckNormal},
        {BEEP_VOLUME, Mi_3, OneEighth, ckNormal}, {0, 0, 18, ckNormal},
        {BEEP_VOLUME, Fa_3, OneEighth, ckNormal}, {0, 0, 18, ckNormal},
        {BEEP_VOLUME, Mi_3, OneFourth, ckNormal}, {0, 0, 18, ckNormal},
        {BEEP_VOLUME, Do_3, OneEighth, ckNormal}, {0, 0, 18, ckNormal},
        {BEEP_VOLUME, Do_3, OneFourth, ckNormal}, {0, 0, 18, ckNormal},
        {BEEP_VOLUME, La_2, OneEighth, ckNormal}, {0, 0, 18, ckNormal},
        {BEEP_VOLUME, Re_3, OneHalfth, ckStop},
};
#define COMPLETED_DURATION_MS (OneHalfth + OneEighth + OneEighth + OneFourth + OneEighth + OneFourth + OneEighth + OneHalfth + 540)

const BeepChunk_t BeepAutodocExhausted[] = {
        {BEEP_VOLUME, Sol_3, OneEighth, ckNormal}, {0, 0, 18, ckNormal},
        {BEEP_VOLUME, Sol_3, OneEighth, ckNormal}, {0, 0, 18, ckNormal},
        {BEEP_VOLUME, Sol_3, OneEighth, ckNormal}, {0, 0, 18, ckNormal},
        {BEEP_VOLUME, Re_D_3, OneWhole, ckStop},
};
#define EXHAUSTED_DURATION_MS 2450    // Sum of sound durations + some time

// Health states
const BeepChunk_t BeepDeath[] = {
        {BEEP_VOLUME, Si_3, 2000, ckNormal},
        {0, 0, 10000, ckStop},
};
const BeepChunk_t BeepRedFast[] = {
        {BEEP_VOLUME, Si_3, 54, ckNormal},
        {0, 0, 54, ckStop},
};

// Emp grenade radiating
const BeepChunk_t BeepGrenade[] = {
        {BEEP_VOLUME, Fa_3,   OneEighth, ckNormal},
        {BEEP_VOLUME, Si_3,   OneFourth, ckStop},
};

// Emp grenade discharged
const BeepChunk_t BeepGrenadeError[] = {
        {BEEP_VOLUME, Sol_D_3, OneSixteenth, ckNormal},
        {BEEP_VOLUME, Sol_3,   OneSixteenth, ckNormal},
        {BEEP_VOLUME, Fa_D_3,  OneSixteenth, ckNormal},
        {BEEP_VOLUME, Fa_3,    OneEighth,    ckStop},
};

// EmpMech
const BeepChunk_t BeepMechBroken   = {BEEP_VOLUME, Sol_D_3, 2007, ckStop};
const BeepChunk_t BeepMechRepaired = {BEEP_VOLUME, Si_3,    2007, ckStop};

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
#define T_SHORT_BLINK_MS    45
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
        {clBlack,  0, clBlack, 0,  nullptr},      // No Pill
        {clBlue, 702, clBlack, 504, BeepPillOk},  // Pill Good
        {clRed,  702, clBlack, 504, BeepPillBad}, // Pill Bad
};

// Health states
const BlinkBeep_t BBHealth[] = {
        {clGreen,  36, clBlack, 3006, nullptr},   // hsGreen
        {clYellow, 36, clBlack, 3006, nullptr},   // hsYellow
        {clRed,    36, clBlack, 1008, nullptr},   // hsRedSlow
        {clRed,    36, clBlack, 54,   BeepShort}, // hsRedFast
        {clRed,    36, clRed,   9999, BeepDeath}, // hsDeath
};

// Battery
#define T_BATTERY_BLINK_MS  36

// Bad ID
const BlinkBeep_t BB_BadID  = {clRed, 450, clBlack, 504, BeepShort};

// Autodoc
const BlinkBeep_t BB_ADInProgress = {clBlue,   54,   clBlack, 180};
const BlinkBeep_t BB_ADCompleted  = {clGreen,  2007, clBlack, COMPLETED_DURATION_MS, BeepAutodocCompleted};
const BlinkBeep_t BB_ADExhausted  = {clYellow, 2007, clBlack, EXHAUSTED_DURATION_MS, BeepAutodocExhausted};

// Detector Fixed
const BlinkBeep_t BB_DmgLevel[] = {
        {clGreen,  270, clBlack, 1440},   // dlClear
        {clYellow, 270, clBlack, 999},    // dlFon
        {clRed,    270, clBlack, 999},    // dlDirty
};

// Emp
const BlinkBeep_t BB_Grenade[] = {
        {clGreen,   180, clBlack, 1044},  // gsReady
        {clBlue,    180, clBlack, 3006},  // gsDischarged
        {clCyan,    180, clBlack, 999 },  // gsCharging
        {clRed,     495, clGreen, 495, BeepGrenade},  // gsRadiating
};

const BlinkBeep_t BB_EmpMech[] = {
        {clGreen,   180, clBlack, 1044},  // msOperational
        {clYellow,  180, clBlack, 1044},  // msRepair
        {clRed,     180, clBlack, 1044},  // msBroken
};

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
