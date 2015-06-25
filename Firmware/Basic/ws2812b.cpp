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
