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

App_t App;

#if 1 // ================================ Pill =================================
void App_t::OnPillConnect() {
    if(PillMgr.Read(PILL_I2C_ADDR, &Pill, sizeof(Pill_t)) != OK) return;
    uint8_t rslt;
//    Uart.Printf("Pill: %u, %u\r", Pill.TypeID, Pill.DeviceID);
    Radio.Enabled = false;
    switch(Pill.TypeID) {
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
            break;

        default:
            Uart.Printf("Unknown Pill: 0x%04X\r", Pill.TypeID);
            Beeper.Beep(BeepPillBad);
            break;
    } // switch
    chThdSleepMilliseconds(1800);
    Radio.Enabled = true;
}
#endif

#if 1 // ======================= Command processing ============================
void App_t::OnUartCmd(uint8_t CmdCode, uint8_t *PData, uint32_t Length) {
    uint8_t b, b2;
    uint16_t w;
    switch(CmdCode) {
        case CMD_PING: Uart.Ack(OK); break;

#if 1 // ==== ID ====
        case CMD_SET_ID:
            if(Length == 2) {
                w = (PData[0] << 8) | PData[1];
                b = ISetID(w);
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
        default: Uart.Ack(CMD_ERROR); break;
    } // switch
}
#endif


#if 1 // ========================= Application =================================
void App_t::Init() {
    // Read device ID and type
    ID = EE.Read32(EE_DEVICE_ID_ADDR);
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
