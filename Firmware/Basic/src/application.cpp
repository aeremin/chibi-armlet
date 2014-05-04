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
#include <cstring>  // for memcpy

App_t App;

#if 1 // ================================ Pill =================================
void App_t::OnPillConnect() {
    if(PillMgr.Read(PILL_I2C_ADDR, &Pill, sizeof(Pill_t)) != OK) return;
    uint8_t rslt;
//    Uart.Printf("Pill: %u, %u\r", Pill.TypeID, Pill.DeviceID);
    switch(Pill.TypeID) {
#if 1 // ==== Set ID ====
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

#if 1 // ==== Cure ====
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

#if 1 // ==== Set consts ====
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
}
#endif

#if 1 // ======================= Command processing ============================
void App_t::OnUartCmd(uint8_t CmdCode, uint8_t *PData, uint32_t Length) {
    uint8_t b, b2;
    uint16_t *p16;
    uint32_t *p32;

    switch(CmdCode) {
        case CMD_PING: Uart.Ack(OK); break;

#if 1 // ==== ID ====
        case CMD_SET_ID:
            if(Length == 2) {
                p16 = (uint16_t*)PData;
                b = ISetID(*p16);
                Uart.Ack(b);
            }
            else Uart.Ack(CMD_ERROR);
            break;
        case CMD_GET_ID:
            UartRplBuf[0] = (ID >> 8) & 0xFF;
            UartRplBuf[1] = ID & 0xFF;
            Uart.Cmd(RPL_GET_ID, UartRplBuf, 2);
            break;
#endif

#if 1 // ==== DoseTop ====
        case CMD_SET_DOSETOP:
            if(Length == sizeof(Dose.Consts.Top)) {
                p32 = (uint32_t*)PData;
                Dose.Consts.Setup(*p32);
                SaveDoseTop();
                Uart.Printf("Top=%u; Red=%u; Yellow=%u\r", Dose.Consts.Top, Dose.Consts.Red, Dose.Consts.Yellow);
            }
            else Uart.Ack(CMD_ERROR);
            break;
        case CMD_GET_DOSETOP:
            p32 = (uint32_t*)UartRplBuf;
            *p32 = Dose.Consts.Top;
            Uart.Cmd(RPL_GET_DOSETOP, UartRplBuf, sizeof(Dose.Consts.Top));
            break;
#endif

#if 1 // ==== Pills ====
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

#if 1 // ==== Dose ====
        case CMD_DOSE_GET:
            p32 = (uint32_t*)UartRplBuf;
            *p32 = Dose.Get();
            Uart.Cmd(RPL_DOSE_GET, UartRplBuf, 4);
            break;
        case CMD_DOSE_SET:
            p32 = (uint32_t*)PData;
            if(*p32 <= Dose.Consts.Top) {
                Dose.Set(*p32, diAlwaysIndicate);
                Uart.Ack(OK);
            }
            else Uart.Ack(FAILURE);
            break;
#endif

        default: Uart.Ack(CMD_ERROR); break;
    } // switch
}
#endif

// ==== Dose ====
void App_t::OnDoseIncTime() {
    // Check if radio damage occured. Will return 1 if no.
    uint32_t FDamage = 1; //Radio.Damage;
    //if(FDamage != 1) Uart.Printf("Dmg=%u\r", FDamage);
    Dose.Increase(FDamage, diUsual);
    //Uart.Printf("Dz=%u; Dmg=%u\r", Dose.Get(), FDamage);
}
void App_t::OnDoseStoreTime() {
    //if(Dose.Save() != OK) Uart.Printf("EE Fail\r");   // disabled for DEBUG
}

#if 1 // ========================= Application =================================
void App_t::Init() {
    ID = EE.Read32(EE_DEVICE_ID_ADDR);  // Read device ID
    // Read dose constants
    uint32_t FTop = EE.Read32(EE_DOSETOP_ADDR);
    if(FTop == 0) FTop = DOSE_DEFAULT_TOP;  // In case of clear EEPROM, use default value
    Dose.Consts.Setup(FTop);
    Uart.Printf("ID=%u; Top=%u; Red=%u; Yellow=%u\r", ID, Dose.Consts.Top, Dose.Consts.Red, Dose.Consts.Yellow);

    //Dose.Load();  // DEBUG
}
uint8_t App_t::ISetID(uint32_t NewID) {
    uint8_t rslt = EE.Write32(EE_DEVICE_ID_ADDR, NewID);
    if(rslt == OK) {
        ID = NewID;
        Uart.Printf("New ID: %u\r", ID);
        return OK;
    }
    else {
        Uart.Printf("EE error: %u\r", rslt);
        return FAILURE;
    }
}
#endif
