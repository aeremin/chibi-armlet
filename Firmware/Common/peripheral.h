/*
 * peripheral.h
 *
 *  Created on: 18.01.2013
 *      Author: kreyl
 */

#ifndef PERIPHERAL_H_
#define PERIPHERAL_H_

#include <stdint.h>
#include "kl_lib_L15x.h"

#if 1 // ================================= Beep ================================
struct BeepChunk_t {
    int8_t VolumePercent;   // 0 means silence, 1...100 means volume, -1 means end
    uint16_t Time_ms;
    uint16_t Freq_Hz;
} PACKED;
#define BEEP_CHUNK_SZ   sizeof(BeepChunk_t)
class Beeper_t {
private:
    VirtualTimer ITmr;
public:
    void BeepI(const BeepChunk_t *PSequence);
    void Beep(const BeepChunk_t *PSequence) {   // Beep with this function
        chSysLock();
        BeepI(PSequence);
        chSysUnlock();
    }
    void Beep(uint32_t ms);
    void Init();
    void Shutdown();
    // Inner use
    PwmPin_t IPin;
};
extern Beeper_t Beeper;
#endif

#if 1 // ============================== LED RGB ================================
// Colors
struct Color_t {
    uint8_t Red, Green, Blue;
    bool operator == (const Color_t AColor) { return ((Red == AColor.Red) and (Green == AColor.Green) and (Blue == AColor.Blue)); }
    //bool operator != (const Color_t AColor) { return ((this->Red != AColor.Red) || (this->Green != AColor.Green) || (this->Blue != AColor.Blue)); }
};
#define clBlack     ((Color_t){0,   0,   0})
#define clRed       ((Color_t){255, 0,   0})
#define clGreen     ((Color_t){0,   255, 0})
#define clBlue      ((Color_t){0,   0,   255})
#define clYellow    ((Color_t){255, 255, 0})
#define clViolet    ((Color_t){255, 0, 255})
#define clCyan      ((Color_t){0, 255, 255})
#define clWhite     ((Color_t){255, 255, 255})

// Timer
#define LED_TIM         TIM3
#define LED_RED_CCR     CCR2
#define LED_GREEN_CCR   CCR4
#define LED_BLUE_CCR    CCR3
#define LED_RCC_EN()    rccEnableTIM3(FALSE)
#define LED_ALTERFUNC   AF2 // TIM3
// GPIO
#define LED_GPIO        GPIOB
#define LED_P1          0   // }
#define LED_P2          1   // }
#define LED_P3          5   // } No need to diff between colors

enum ChunkKind_t {ckNormal=0, ckLast=1};
struct LedChunk_t {
    Color_t Color;
    uint16_t Time_ms;
    ChunkKind_t ChunkKind;
} PACKED;
#define LED_CHUNK_SZ   sizeof(LedChunk_t)

class LedRGB_t {
private:
    const LedChunk_t *IPFirstChunk;
    VirtualTimer ITmr;
    void ISetRed  (uint8_t AValue) {LED_TIM->LED_RED_CCR   = AValue;}
    void ISetGreen(uint8_t AValue) {LED_TIM->LED_GREEN_CCR = AValue;}
    void ISetBlue (uint8_t AValue) {LED_TIM->LED_BLUE_CCR  = AValue;}
public:
    void Init();
    void SetColor(Color_t AColor) {
        ISetRed(AColor.Red);
        ISetGreen(AColor.Green);
        ISetBlue(AColor.Blue);
    }
    void StartBlink(const LedChunk_t *PLedChunk) {
        chSysLock();
        IPFirstChunk = PLedChunk; // Save first chunk
        IStartBlinkI(PLedChunk);
        chSysUnlock();
    }
    void StopBlink() {
        chSysLock();
        if(chVTIsArmedI(&ITmr)) chVTResetI(&ITmr);
        SetColor(clBlack);
        chSysUnlock();
    }
    // Inner use
    void IStartBlinkI(const LedChunk_t *PLedChunk);
};

extern LedRGB_t Led;
#endif

#endif /* PERIPHERAL_H_ */
