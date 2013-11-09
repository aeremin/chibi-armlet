/*
 * application.cpp
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#include "application.h"
#include "cmd_uart.h"

App_t App;

void App_t::Init() {

}

#if 1 // ======================= Command processing ============================
void UartCmdCallback(uint8_t CmdCode, uint8_t *PData, uint32_t Length) {
    uint8_t b;
    switch(CmdCode) {
        case 0x01:
            b = OK;
            Uart.Cmd(0x90, &b, 1);
            break;

        case 0x51:  // GetID
            Uart.Printf("ID=%u\r", App.ID);
            break;

        case 0x52:  // SetID
            App.ID = *((uint16_t*)PData);
            Uart.Printf("New ID=%u\r", App.ID);
            break;

        default: break;
    } // switch
}
#endif
