/*
 * ws2812b.cpp
 *
 *  Created on: 05 апр. 2014 г.
 *      Author: Kreyl
 */

#include "ws2812b.h"
#include "uart.h"

extern "C" {
// Wrapper for Tx Completed IRQ
void LedTxcIrq(void *p, uint32_t flags) {
    dmaStreamDisable(LED_DMA_STREAM);
    LedWs.IStopTx();
//    Uart.PrintfI("\rIrq\r");
}
static inline void LedTmrCallback(void *p) { LedWs.ITmrHandler(); }
} // "C"

void LedWs_t::Init() {
    // ==== Timer ====
    // Init tmr in PWM mode
    TxTmr.Init();
    TxTmr.Set(0);
    TxTmr.EnableDmaOnUpdate();
    TxTmr.Disable();

    // ==== DMA ====
    dmaStreamAllocate     (LED_DMA_STREAM, IRQ_PRIO_LOW, LedTxcIrq, NULL);
    dmaStreamSetPeripheral(LED_DMA_STREAM, TMR_PCCR(LED_TMR, LED_TMR_CHNL));
    dmaStreamSetMode      (LED_DMA_STREAM, LED_DMA_MODE);
}

void LedWs_t::SetCommonColor(Color_t Clr) {
    for(uint32_t i=0; i<LED_CNT; i++) IClr[i] = Clr;
    ISetCurrentColors();
}

void LedWs_t::AppendBitsMadeOfByte(uint8_t Byte) {
    for(uint8_t i=0; i<8; i++) {
        if(Byte & 0x80) *PBit = WS_T1H_N;
        else *PBit = WS_T0H_N;
        PBit++;
        Byte <<= 1;
    }
}

void LedWs_t::ISetCurrentColors() {
    // Fill bit buffer
    PBit = &BitBuf[RST_BIT_CNT];
    for(uint32_t i=0; i<LED_CNT; i++) {
        AppendBitsMadeOfByte(IClr[i].G);
        AppendBitsMadeOfByte(IClr[i].R);
        AppendBitsMadeOfByte(IClr[i].B);
    }
//    Uart.Printf("\r ");
//    for(uint32_t i=0; i<TOTAL_BIT_CNT; i++) Uart.Printf(" %u", BitBuf[i]);

    // Start transmission
    dmaStreamSetMemory0(LED_DMA_STREAM, BitBuf);
    dmaStreamSetTransactionSize(LED_DMA_STREAM, TOTAL_BIT_CNT);
    dmaStreamSetMode(LED_DMA_STREAM, LED_DMA_MODE);
    dmaStreamEnable(LED_DMA_STREAM);
    TxTmr.SetCounter(0);

//    chSysLockFromIsr();
    TxTmr.Enable();
//    dmaWaitCompletion(LED_DMA_STREAM);
//    IStopTx();
//    chSysUnlockFromIsr();
}

SequencerLoopTask_t LedWs_t::ISetup() {
    int N = -1;
    // Search LED with other color
    for(int i=0; i<LED_CNT; i++) {
        if(IClr[i] != IPCurrentChunk->Color) {
            N = i;
            break;
        }
    }
    if(N == -1) IPCurrentChunk++;   // All are equal
    else {
        if(IPCurrentChunk->Value == 0) {     // If smooth time is zero,
            SetCommonColor(IPCurrentChunk->Color); // set all colors now
            IPCurrentChunk++;                // and goto next chunk
        }
        else { // Smooth
            uint32_t Delay = 0, tmp;
            if(IPCurrentChunk->SwType == wstAtOnce) {
                // Adjust all colors, calculating max delay
                for(int i=N; i<LED_CNT; i++) {
                    IClr[i].Adjust(&IPCurrentChunk->Color);
                    tmp = IClr[i].SmoothDelay(IPCurrentChunk->Color, IPCurrentChunk->Value);
                    if(tmp > Delay) Delay = tmp;
                }
                ISetCurrentColors();
                if(Delay == 0) IPCurrentChunk++;    // All done
                else {                              // Not completed
                    SetupDelay(Delay);
                    return sltBreak;
                } // Done or not
            }
            else { // One by one
                // Adjust current color
                IClr[N].Adjust(&IPCurrentChunk->Color);
                ISetCurrentColors();
                Delay = IClr[N].SmoothDelay(IPCurrentChunk->Color, IPCurrentChunk->Value);
                if(Delay == 0) {
                    if(N == LED_CNT-1) { IPCurrentChunk++; return sltProceed; } // Last LED
                    else SetupDelay(7);   // Current LED completed, wait a little and proceed with next
                }
                else SetupDelay(Delay);   // Wait as needed and proceed
                return sltBreak;
            } // One by one
        } // if smooth
    } // If there is someone other
    return sltProceed;
}
