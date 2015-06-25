/*
 * ws2812b.h
 *
 *  Created on: 05 апр. 2014 г.
 *      Author: Kreyl
 */

#ifndef WS2812B_H_
#define WS2812B_H_

#include "kl_lib.h"
#include "Sequences.h"

// Constants
#define LED_CNT         3
#define LED_TMR         TIM4
#define LED_TMR_CHNL    1
#define LED_GPIO        GPIOB
#define LED_PIN         6
#define LED_DMA_STREAM  STM32_DMA1_STREAM7  // TIM4_UP, intersects with pill

#define LED_DMA_MODE    DMA_PRIORITY_VERYHIGH \
                        | STM32_DMA_CR_MSIZE_BYTE \
                        | STM32_DMA_CR_PSIZE_HWORD \
                        | STM32_DMA_CR_MINC     /* Memory pointer increase */ \
                        | STM32_DMA_CR_DIR_M2P  /* Direction is memory to peripheral */ \
                        | STM32_DMA_CR_TCIE     /* Enable Transmission Complete IRQ */

// Bit Buffer
#define RST_BIT_CNT     45 // 45 zero bits to produce reset
#define DATA_BIT_CNT    (LED_CNT * 3 * 8)   // 3 channels 8 bit each
#define END_0_BIT_CNT   16
#define TOTAL_BIT_CNT   (RST_BIT_CNT + DATA_BIT_CNT + END_0_BIT_CNT)

// Tx timings
#define WS_T0H_N        1
#define WS_T1H_N        3
#define WS_T_TOPVALUE   18

class LedWs_t : public BaseSequencer_t<LedWsChunk_t> {
private:
    PinOutputPWM_t<WS_T_TOPVALUE, invNotInverted, omPushPull> TxTmr;
    uint8_t BitBuf[TOTAL_BIT_CNT], *PBit, Indx;
    Color_t IClr[LED_CNT];
    void AppendBitsMadeOfByte(uint8_t Byte);
    void ISetCurrentColors();
    uint32_t ICalcDelay(uint32_t CurrentBrightness, uint32_t SmoothVar) { return (uint32_t)((SmoothVar / (CurrentBrightness+4)) + 1); }
    void ISwitchOff() { SetCommonColor(clBlack); }
    SequencerLoopTask_t ISetup();
public:
    LedWs_t() : BaseSequencer_t(), TxTmr({LED_GPIO, LED_PIN, LED_TMR, LED_TMR_CHNL}), PBit(BitBuf), Indx(0) {
        for(uint32_t i=0; i<TOTAL_BIT_CNT; i++) BitBuf[i] = 0;
    }
    void Init();
    void SetCommonColor(Color_t Clr);
    // Inner use
    void IStopTx() { TxTmr.Set(0); TxTmr.Disable(); }
    void ITmrHandler();
};

extern LedWs_t LedWs;

#endif /* WS2812B_H_ */
