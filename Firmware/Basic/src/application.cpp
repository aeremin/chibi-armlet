/*
 * application.cpp
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#include "application.h"
#include "cmd_uart.h"
#include "pill.h"
#include "peripheral.h"
#include "sequences.h"

App_t App;
static uint8_t SBuf[252];

#if 1 // ================================ Pill =================================
struct Med_t {
    uint8_t CureID, Charges;
} __attribute__ ((__packed__));
static Med_t Med;
#endif

#if 1 // ========================= Application =================================
static WORKING_AREA(waAppThread, 256);
__attribute__((noreturn))
static void AppThread(void *arg) {
    chRegSetThreadName("App");

    while(1) {
        chThdSleepMilliseconds(999);
        // Check pills
        PillChecker();
        if(PillsHaveChanged) {  // Will be reset at PillChecker
            Beeper.Beep(ShortBeep);
            // Read med
            if(Pill[0].Connected) {
                Pill[0].Read((uint8_t*)&Med, sizeof(Med_t));
                Uart.Printf("Pill: %u, %u\r", Med.CureID, Med.Charges);
            }
        } // if pill changed

    } // while 1
}

void App_t::Init() {
    chThdCreateStatic(waAppThread, sizeof(waAppThread), NORMALPRIO, (tfunc_t)AppThread, NULL);
}
#endif

#if 1 // ======================= Command processing ============================
void Ack(uint8_t Result) { Uart.Cmd(0x90, &Result, 1); }

void UartCmdCallback(uint8_t CmdCode, uint8_t *PData, uint32_t Length) {
    uint8_t b, b2;
    switch(CmdCode) {
        case CMD_PING: Ack(OK); break;

        case 0x51:  // GetID
            Uart.Printf("ID=%u\r", App.ID);
            break;

        case 0x52:  // SetID
            App.ID = *((uint16_t*)PData);
            Uart.Printf("New ID=%u\r", App.ID);
            break;

        // ==== Pills ====
        case CMD_PILL_STATE:
            b = PData[0];   // Pill address
            if(b <= 7) SBuf[1] = Pill[b].CheckIfConnected();
            SBuf[0] = b;
            Uart.Cmd(RPL_PILL_STATE, SBuf, 2);
            break;
        case CMD_PILL_WRITE:
            b = PData[0];
            if(b <= 7) SBuf[1] = Pill[b].Write(&PData[1], Length-1);
            SBuf[0] = b;
            Uart.Cmd(RPL_PILL_WRITE, SBuf, 2);
            break;
        case CMD_PILL_READ:
            b = PData[0];           // Pill address
            b2 = PData[1];          // Data size to read
            if(b2 > 250) b2 = 250;  // Check data size
            if(b <= 7) SBuf[1] = Pill[b].Read(&SBuf[2], b2);
            SBuf[0] = b;
            if(SBuf[1] == OK) Uart.Cmd(RPL_PILL_READ, SBuf, b2+2);
            else Uart.Cmd(RPL_PILL_READ, SBuf, 2);
            break;

        default: break;
    } // switch
}
#endif
