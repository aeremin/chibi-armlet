/*
 * cmd_uart.cpp
 *
 *  Created on: 15.04.2013
 *      Author: kreyl
 */

#include "MsgQ.h"
#include <string.h>
#include "uart.h"
#include "kl_lib.h"

#if 1 // ==================== Common and eternal ===============================
// Pins Alternate function
#if defined STM32L4XX || defined STM32F0XX
#define UART_TX_REG     TDR
#define UART_RX_REG     RDR
#elif defined STM32L1XX || defined STM32F2XX || defined STM32F1XX
#define UART_TX_REG     DR
#define UART_RX_REG     DR
#else
#error "Not defined"
#endif

// Array of utilized UARTs to organize RX
//static BaseUart_t* PUarts[UARTS_CNT];
#endif // Common and eternal

#if 1 // ========================= Base UART ===================================
#if 1 // ==== TX ====

#if UART_USE_DMA
// Wrapper for TX IRQ
extern "C"
void DmaUartTxIrq(void *p, uint32_t flags) { ((BaseUart_t*)p)->IRQDmaTxHandler(); }

// ==== TX DMA IRQ ====
void BaseUart_t::IRQDmaTxHandler() {
    dmaStreamDisable(PDmaTx);    // Registers may be changed only when stream is disabled
    IFullSlotsCount -= ITransSize;
    PRead += ITransSize;
    if(PRead >= (TXBuf + UART_TXBUF_SZ)) PRead = TXBuf; // Circulate pointer

    if(IFullSlotsCount == 0) {  // Nothing left to send
        IDmaIsIdle = true;
        IOnTxEnd();
    }
    else ISendViaDMA();
}

void BaseUart_t::ISendViaDMA() {
    uint32_t PartSz = (TXBuf + UART_TXBUF_SZ) - PRead; // Cnt from PRead to end of buf
    ITransSize = MIN_(IFullSlotsCount, PartSz);
    if(ITransSize != 0) {
        IDmaIsIdle = false;
        dmaStreamSetMemory0(PDmaTx, PRead);
        dmaStreamSetTransactionSize(PDmaTx, ITransSize);
        dmaStreamSetMode(PDmaTx, Params->DmaModeTx);
        dmaStreamEnable(PDmaTx);
    }
}

uint8_t BaseUart_t::IPutByte(uint8_t b) {
    if(IFullSlotsCount >= UART_TXBUF_SZ) return retvOverflow;
    *PWrite++ = b;
    if(PWrite >= &TXBuf[UART_TXBUF_SZ]) PWrite = TXBuf;   // Circulate buffer
    IFullSlotsCount++;
    return retvOk;
}

void BaseUart_t::IStartTransmissionIfNotYet() {
    if(IDmaIsIdle) ISendViaDMA();
}
#else // DMA not used
uint8_t Uart_t::IPutChar(char c) {
    return IPutCharNow(c);
}
void Uart_t::IStartTransmissionIfNotYet() { }
#endif

uint8_t BaseUart_t::IPutByteNow(uint8_t b) {
#if defined STM32L1XX || defined STM32F2XX || defined STM32F4XX || defined STM32F10X_LD_VL
    while(!(Params->Uart->SR & USART_SR_TXE));
    Params->Uart->UART_TX_REG = b;
    while(!(Params->Uart->SR & USART_SR_TXE));
#elif defined STM32F0XX || defined STM32L4XX
    while(!(Params->Uart->ISR & USART_ISR_TXE));
    Params->Uart->UART_TX_REG = b;
    while(!(Params->Uart->ISR & USART_ISR_TXE));
#endif
    return retvOk;
}
#endif // TX

#if 1 // ==== Init ====
void BaseUart_t::Init() {
    AlterFunc_t PinAF = AF1;
    // ==== Tx pin ====
#if defined STM32L4XX || defined STM32L1XX || defined STM32F2XX
    PinAF = AF7;
#if defined UART4
    if(Params->Uart == UART4) PinAF = AF8;
#endif
#if defined UART5
    if(Params->Uart == UART5) PinAF = AF8;
#endif
#if defined USART6
    if(Params->Uart == USART6) PinAF = AF8;
#endif

#elif defined STM32F0XX
    if(Params->PGpioTx == GPIOA) PinAF = AF1;
    else if(Params->PGpioTx == GPIOB) PinAF = AF0;
#elif defined STM32F1XX
    // Do nothing as F1xx does not use AF number
#else
#error "UART AF not defined"
#endif
    PinSetupAlterFunc(Params->PGpioTx, Params->PinTx, omPushPull, pudNone, PinAF);
    // ==== Clock ====
    if     (Params->Uart == USART1) { rccEnableUSART1(FALSE); }
    else if(Params->Uart == USART2) { rccEnableUSART2(FALSE); }
#if defined USART3
    else if(Params->Uart == USART3) { rccEnableUSART3(FALSE); }
#endif
#if defined UART4
    else if(Params->Uart == UART4) { rccEnableUART4(FALSE); }
#endif
#if defined UART5
    else if(Params->Uart == UART5) { rccEnableUART5(FALSE); }
#endif
#if defined USART6
    else if(Params->Uart == USART6) { rccEnableUSART6(FALSE); }
#endif
    // Setup independent clock if possible and required
#if defined STM32F072xB
    if(Params->UseIndependedClock) {
        Clk.EnableHSI();    // HSI used as independent clock
        if     (Params->Uart == USART1) RCC->CFGR3 |= RCC_CFGR3_USART1SW_HSI;
        else if(Params->Uart == USART2) RCC->CFGR3 |= RCC_CFGR3_USART2SW_HSI;
    }
#elif defined STM32L4XX
    if(Params->UseIndependedClock) {
        Clk.EnableHSI();    // HSI used as independent clock
        if     (Params->Uart == USART1) RCC->CCIPR |= 0b10;
        else if(Params->Uart == USART2) RCC->CCIPR |= 0b10 << 2;
        else if(Params->Uart == USART3) RCC->CCIPR |= 0b10 << 4;
#ifdef UART4
        else if(Params->Uart == UART4)  RCC->CCIPR |= 0b10 << 6;
#endif
#ifdef UART5
        else if(Params->Uart == UART5)  RCC->CCIPR |= 0b10 << 8;
#endif
    }
#endif
    OnClkChange();  // Setup baudrate

    Params->Uart->CR2 = 0;  // Nothing that interesting there
#if UART_USE_DMA    // ==== DMA ====
    // Remap DMA request if needed
#if defined STM32F0XX
    if(Params->PDmaTx == STM32_DMA1_STREAM4) SYSCFG->CFGR1 |= SYSCFG_CFGR1_USART1TX_DMA_RMP;
#endif
    PDmaTx = dmaStreamAlloc(Params->DmaTxID, IRQ_PRIO_MEDIUM, DmaUartTxIrq, this);
    dmaStreamSetPeripheral(PDmaTx, &Params->Uart->UART_TX_REG);
    dmaStreamSetMode      (PDmaTx, Params->DmaModeTx);
    IDmaIsIdle = true;
#endif

    Params->Uart->CR1 = USART_CR1_TE;
    Params->Uart->CR3 = USART_CR3_DMAT;
    Params->Uart->CR1 |= USART_CR1_UE;    // Enable USART
}

void BaseUart_t::Shutdown() {
    Params->Uart->CR1 &= ~USART_CR1_UE; // UART Disable
    if     (Params->Uart == USART1) { rccDisableUSART1(); }
    else if(Params->Uart == USART2) { rccDisableUSART2(); }
#if defined USART3
    else if(Params->Uart == USART3) { rccDisableUSART3(); }
#endif
#if defined UART4
    else if(Params->Uart == UART4) { rccDisableUART4(FALSE); }
#endif
#if defined UART5
    else if(Params->Uart == UART5) { rccDisableUART5(FALSE); }
#endif
}

void BaseUart_t::OnClkChange() {
#if defined STM32L1XX || defined STM32F1XX
    if(Params->Uart == USART1) Params->Uart->BRR = Clk.APB2FreqHz / Params->Baudrate;
    else                       Params->Uart->BRR = Clk.APB1FreqHz / Params->Baudrate;
#elif defined STM32F072xB
    if(Params->Uart == USART1 or Params->Uart == USART2) Params->Uart->BRR = HSI_FREQ_HZ / Params->Baudrate;
    else Params->Uart->BRR = Clk.APBFreqHz / Params->Baudrate;
#elif defined STM32F0XX
    Params->Uart->BRR = Clk.APBFreqHz / IBaudrate;
#elif defined STM32F2XX || defined STM32F4XX
    if(Params->Uart == USART1 or Params->Uart == USART6) Params->Uart->BRR = Clk.APB2FreqHz / IBaudrate;
    else Params->Uart->BRR = Clk.APB1FreqHz / IBaudrate;
#elif defined STM32L4XX
    if(Params->UseIndependedClock) Params->Uart->BRR = HSI_FREQ_HZ / Params->Baudrate;
    else {
        if(Params->Uart == USART1) Params->Uart->BRR = Clk.APB2FreqHz / Params->Baudrate;
        else Params->Uart->BRR = Clk.APB1FreqHz / Params->Baudrate; // All others at APB1
    }
#endif
}
#endif // Init

#endif // Base UART
