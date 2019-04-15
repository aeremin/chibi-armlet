/*
 * cmd_uart.h
 *
 *  Created on: 15.04.2013
 *      Author: kreyl
 */

#pragma once

#include "kl_lib.h"
#include <cstring>
#include "shell.h"
#include "board.h"

extern "C"
void DmaUartTxIrq(void *p, uint32_t flags);

struct UartParams_t {
    uint32_t Baudrate;
    USART_TypeDef* Uart;
    GPIO_TypeDef *PGpioTx;
    uint16_t PinTx;
    // DMA
    uint32_t DmaTxID;
    uint32_t DmaModeTx;
    UartParams_t(uint32_t ABaudrate, USART_TypeDef* AUart,
            GPIO_TypeDef *APGpioTx, uint16_t APinTx,
            uint32_t ADmaTxID, uint32_t ADmaModeTx
    ) : Baudrate(ABaudrate), Uart(AUart),
            PGpioTx(APGpioTx), PinTx(APinTx),
            DmaTxID(ADmaTxID), DmaModeTx(ADmaModeTx) {}
};

#define UART_USE_DMA        TRUE
#define UART_USE_TXE_IRQ    FALSE

#define UART_CMD_BUF_SZ     54 // payload bytes
#define UART_RX_POLLING_MS  99

// ==== Base class ====
class BaseUart_t {
protected:
    const stm32_dma_stream_t *PDmaTx;
    const UartParams_t *Params;
#if UART_USE_DMA
    char TXBuf[UART_TXBUF_SZ];
    char *PRead, *PWrite;
    bool IDmaIsIdle;
    uint32_t IFullSlotsCount, ITransSize;
    void ISendViaDMA();
#endif
    int32_t OldWIndx, RIndx;
protected:
    uint8_t IPutByte(uint8_t b);
    uint8_t IPutByteNow(uint8_t b);
    void IStartTransmissionIfNotYet();
    virtual void IOnTxEnd() = 0;
    // ==== Constructor ====
    BaseUart_t(const UartParams_t *APParams) : Params(APParams)
#if UART_USE_DMA
    , PRead(TXBuf), PWrite(TXBuf), IDmaIsIdle(true), IFullSlotsCount(0), ITransSize(0)
#endif
    , OldWIndx(0), RIndx(0)
    {}
public:
    void Init();
    void Shutdown();
    void OnClkChange();
    // Enable/Disable
    void EnableTx()  { Params->Uart->CR1 |= USART_CR1_TE; }
    void DisableTx() { Params->Uart->CR1 &= ~USART_CR1_TE; }
#if UART_USE_DMA
    void FlushTx() { while(!IDmaIsIdle) chThdSleepMilliseconds(1); }  // wait DMA
#endif
    void EnableTCIrq(const uint32_t Priority, ftVoidVoid ACallback);
#if UART_USE_DMA
    void IRQDmaTxHandler();
#endif
};

class CmdUart_t : public BaseUart_t, public PrintfHelper_t, public Shell_t {
private:
    void IOnTxEnd() {} // Dummy
    uint8_t IPutChar(char c) { return IPutByte(c);  }
    void IStartTransmissionIfNotYet() { BaseUart_t::IStartTransmissionIfNotYet(); }
    void Print(const char *format, ...) {
        va_list args;
        va_start(args, format);
        IVsPrintf(format, args);
        va_end(args);
    }
public:
    CmdUart_t(const UartParams_t *APParams) : BaseUart_t(APParams) {}
    void ProcessByteIfReceived();
};
