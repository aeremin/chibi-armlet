/*
 * application.cpp
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#include "application.h"
#include "cmd_uart.h"
#include "pill_mgr.h"
#include "peripheral.h"
#include "evt_mask.h"
#include "eestore.h"
#include "radio_lvl1.h"
#include "adc15x.h"
#include <cstdlib>

App_t App;

#if 1 // ================================ Pill =================================
void App_t::OnPillConnect() {
    if(PillMgr.Read(PILL_I2C_ADDR, &Pill, sizeof(Pill_t)) != OK) return;
    Uart.Printf("Pill: %u\r", Pill.TypeID);
#ifndef DEVTYPE_PILLPROG
    uint8_t rslt;
    switch(Pill.TypeID) {
#if 0 // ==== Set ID ====
        // BEWARE! LittleEndian!
        case PILL_TYPEID_SET_ID:
            if(ID == 0) {
                Pill.DeviceID++;
                rslt = PillMgr.Write(PILL_I2C_ADDR, &Pill, (sizeof(Pill.TypeID) + sizeof(Pill.DeviceID)));
                if(rslt == OK) {
                    ISetID(Pill.DeviceID-1);
                    Led.StartBlink(LedPillIdSet);
                    Beeper.Beep(BeepPillOk);
                }
                else {
                    Uart.Printf("Pill Write Error\r");
                    Led.StartBlink(LedPillIdNotSet);
                    Beeper.Beep(BeepPillBad);
                }
            }
            else {
                Uart.Printf("ID already set: %u\r", ID);
                Led.StartBlink(LedPillIdNotSet);
                Beeper.Beep(BeepPillBad);
            }
            chThdSleepMilliseconds(1800);
            break;
#endif

#ifdef DEVTYPE_UMVOS // ==== Cure ====
        case PILL_TYPEID_CURE:
            switch(Pill.ChargeCnt) {
                case 0: rslt = FAILURE; break;      // Discharged pill
                case INFINITY16: rslt = OK; break;  // Infinite count of charges, do not decrease
                default:
                    Pill.ChargeCnt--;
                    rslt = PillMgr.Write(PILL_I2C_ADDR, (uint8_t*)&Pill, sizeof(Pill_t));
                    break;
            } // switch charge

            if(rslt == OK) {
                Beeper.Beep(BeepPillOk);
                Led.StartBlink(LedPillCureOk);
                // Decrease dose if not dead, or if this is panacea
                if((Dose.State != hsDeath) or (Pill.Value == INFINITY32)) Dose.Decrease(Pill.Value, diNeverIndicate);
                chThdSleepMilliseconds(2007);    // Let indication to complete
                Dose.RenewIndication();
            }
            else {
                Beeper.Beep(BeepPillBad);
                Led.StartBlink(LedPillBad);
                chThdSleepMilliseconds(2007);    // Let indication to complete
            }
            break;
#endif

#ifdef DEVTYPE_UMVOS // ==== Set consts ====
        case PILL_TYPEID_SET_DOSETOP:
            Dose.Consts.Setup(Pill.DoseTop);
            SaveDoseTop();
            Uart.Printf("Top=%u; Red=%u; Yellow=%u\r", Dose.Consts.Top, Dose.Consts.Red, Dose.Consts.Yellow);
            Led.StartBlink(LedPillSetupOk);
            Beeper.Beep(BeepPillOk);
            chThdSleepMilliseconds(2007);
            break;
#endif
        default:
            Uart.Printf("Unknown Pill: 0x%04X\r", Pill.TypeID);
            Beeper.Beep(BeepPillBad);
            break;
    } // switch
#else // DEVTYPE_PILLPROG
    Led.StartBlink(LedPillSetupOk);
    Beeper.Beep(BeepPillOk);
}
#endif
#endif

#if 1 // ======================= Command processing ============================
void App_t::OnUartCmd(Cmd_t *PCmd) {
//    Uart.Printf("%S\r", PCmd->S);
    int32_t d;
    uint8_t b;
    //    uint32_t dw32 __attribute__((unused));  // May be unused in some cofigurations
//    Uart.Printf("%S\r", S);
    if(PCmd->NameIs("#Ping")) Uart.Ack();

#if 1 // ==== ID ====
    else if(PCmd->NameIs("#SetID")) {
        if(PCmd->TryConvertToNumber(&d) == OK) {  // Next token is number
            b = ISetID(d);
            Uart.Ack(b);
        }
        else Uart.Ack(CMD_ERROR);
    }
    else if(PCmd->NameIs("#GetID")) Uart.Printf("#ID %u\r", ID);
#endif

#if 0 // ==== Pills ====
    else if

        case CMD_PILL_STATE:
            b = PData[0];   // Pill address
            if(b <= 7) {
                UartRplBuf[0] = b;
                UartRplBuf[1] = PillMgr.CheckIfConnected(PILL_I2C_ADDR);
                Uart.Cmd(RPL_PILL_STATE, UartRplBuf, 2);
            }
            else Uart.Ack(CMD_ERROR);
            break;
        case CMD_PILL_WRITE:
            b = PData[0];   // Pill address
            if(b <= 7) {
                UartRplBuf[0] = b;    // Pill address
                UartRplBuf[1] = PillMgr.Write(PILL_I2C_ADDR, &PData[1], Length-1); // Write result
                Uart.Cmd(RPL_PILL_WRITE, UartRplBuf, 2);
            }
            else Uart.Ack(CMD_ERROR);
            break;
        case CMD_PILL_READ:
            b = PData[0];           // Pill address
            b2 = PData[1];          // Data size to read
            if(b2 > (UART_RPL_BUF_SZ-2)) b2 = (UART_RPL_BUF_SZ-2);  // Check data size
            if(b <= 7) {
                UartRplBuf[0] = b;                                  // Pill address
                UartRplBuf[1] = PillMgr.Read(PILL_I2C_ADDR, &UartRplBuf[2], b2);    // Read result
                if(UartRplBuf[1] == OK) Uart.Cmd(RPL_PILL_READ, UartRplBuf, b2+2);
                else Uart.Cmd(RPL_PILL_READ, UartRplBuf, 2);
            }
            else Uart.Ack(CMD_ERROR);
            break;
#endif

#ifdef DEVTYPE_UMVOS // ==== DoseTop ====
        case CMD_SET_DOSETOP:
            if(Length == sizeof(Dose.Consts.Top)) {
                dw32 = Convert::ArrToU32AsBE(PData);
                Dose.Consts.Setup(w32);
                SaveDoseTop();
                Uart.Printf("Top=%u; Red=%u; Yellow=%u\r", Dose.Consts.Top, Dose.Consts.Red, Dose.Consts.Yellow);
            }
            else Uart.Ack(CMD_ERROR);
            break;
        case CMD_GET_DOSETOP:
            Convert::U32ToArrAsBE(UartRplBuf, Dose.Consts.Top);
            Uart.Cmd(RPL_GET_DOSETOP, UartRplBuf, sizeof(Dose.Consts.Top));
            break;

        case CMD_DOSE_SET:
            if(Length == sizeof(uint32_t)) {
                dw32 = Convert::ArrToU32AsBE(PData);
                if(dw32 <= Dose.Consts.Top) {
                    Dose.Set(dw32, diAlwaysIndicate);
                    Uart.Ack(OK);
                }
                else Uart.Ack(FAILURE);
            }
            else Uart.Ack(CMD_ERROR);
            break;
        case CMD_DOSE_GET:
            Convert::U32ToArrAsBE(UartRplBuf, Dose.Get());
            Uart.Cmd(RPL_DOSE_GET, UartRplBuf, 4);
            break;
#endif

    else Uart.Ack(CMD_UNKNOWN);
}
#endif

#ifdef DEVTYPE_UMVOS // ==== Dose ====
void App_t::OnDoseIncTime() {
    // Check if radio damage occured. Will return 1 if no.
    uint32_t FDamage = 1; //Radio.Damage;
    //if(FDamage != 1) Uart.Printf("Dmg=%u\r", FDamage);
    Dose.Increase(FDamage, diUsual);
    //Uart.Printf("Dz=%u; Dmg=%u\r", Dose.Get(), FDamage);
}

void App_t::OnRxTableReady() {
    Uart.Printf("RxTbl: %u;  %u\r", RxTable.PTable->Size, chTimeNow());
    RxTable.Print();
}
#endif

#if 1 // ========================= Application =================================
void App_t::Init() {
    ID = EE.Read32(EE_DEVICE_ID_ADDR);  // Read device ID
#ifdef DEVTYPE_UMVOS
    // Read dose constants
    uint32_t FTop = EE.Read32(EE_DOSETOP_ADDR);
    if(FTop == 0) FTop = DOSE_DEFAULT_TOP;  // In case of clear EEPROM, use default value
    Dose.Consts.Setup(FTop);
    Uart.Printf("ID=%u; Top=%u; Red=%u; Yellow=%u\r", ID, Dose.Consts.Top, Dose.Consts.Red, Dose.Consts.Yellow);

    //Dose.Load();  // DEBUG
#endif
}
uint8_t App_t::ISetID(uint32_t NewID) {
    if(NewID > 0xFFFF) return FAILURE;
    uint8_t rslt = EE.Write32(EE_DEVICE_ID_ADDR, NewID);
    if(rslt == OK) {
        ID = NewID;
//        Uart.Printf("New ID: %u\r", ID);
        return OK;
    }
    else {
        Uart.Printf("EE error: %u\r", rslt);
        return FAILURE;
    }
}
#endif
