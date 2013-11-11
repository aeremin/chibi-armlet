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

#if 1 // =============================== Status ================================
#define DOSE_RED_END        60
#define DOSE_RED_FAST       50
#define DOSE_RED_SLOW       40
#define DOSE_YELLOW         20

enum HealthState_t {hsNone=0, hsGreen, hsYellow, hsRedSlow, hsRedFast, hsDeath};
class Dose_t {
private:
    uint32_t IDose;
    void ChangeIndication() {
        Beeper.Stop();
        Led.StopBlink();
        switch(State) {
            case hsDeath:
                Led.SetColor(clRed);
                Beeper.Beep(BeepDeath);
                break;
            case hsRedFast:
                Led.StartBlink(LedRedFast);
                Beeper.Beep(BeepRedFast);
                break;
            case hsRedSlow:
                Led.StartBlink(LedRedSlow);
                Beeper.Beep(BeepBeepLoud);
                break;
            case hsYellow:
                Led.StartBlink(LedYellow);
                Beeper.Beep(BeepBeepLoud);
                break;
            case hsGreen:
                Led.StartBlink(LedGreen);
                Beeper.Beep(BeepBeepLoud);
                break;
            default: break;
        } // switch
    }
    void ConvertDoseToState() {
        if     (IDose >= DOSE_RED_END)  State = hsDeath;
        else if(IDose >= DOSE_RED_FAST) State = hsRedFast;
        else if(IDose >= DOSE_RED_SLOW) State = hsRedSlow;
        else if(IDose >= DOSE_YELLOW)   State = hsYellow;
        else                            State = hsGreen;
    }
public:
    HealthState_t State;
    void Set(uint32_t ADose) {
        IDose = ADose;
        HealthState_t OldState = State;
        ConvertDoseToState();
        if(State != OldState) ChangeIndication();
    }
    uint32_t Get() { return IDose; }
    void Increase(uint32_t Amount) {
        uint32_t Dz = IDose;
        if((Dz + Amount) > DOSE_RED_END) Dz = DOSE_RED_END;
        else Dz += Amount;
        Set(Dz);
    }
};
static Dose_t Dose;
#endif

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
        // ==== Check pills ====
        PillChecker();
        if(PillsHaveChanged) {  // Will be reset at PillChecker
            Beeper.Beep(ShortBeep);
            // Read med
            if(Pill[0].Connected) {
                Pill[0].Read((uint8_t*)&Med, sizeof(Med_t));
                Uart.Printf("Pill: %u, %u\r", Med.CureID, Med.Charges);
            }
        } // if pill changed

        // ==== Process dose ====
        Dose.Increase(1);


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
