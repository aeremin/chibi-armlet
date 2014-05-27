/*
 * application.cpp
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#include "application.h"
#include "colors_sounds.h"
#include "cmd_uart.h"
#include "evt_mask.h"
#include "radio_lvl1.h"
#include <cstdlib>

App_t App;

#if 0 // ======================= Command processing ============================
void App_t::OnUartCmd(Cmd_t *PCmd) {
//    Uart.Printf("%S\r", PCmd->Name);
    uint8_t b;
    uint32_t dw32 __attribute__((unused));  // May be unused in some cofigurations
    if(PCmd->NameIs("#Ping")) Uart.Ack(OK);
}
#endif
