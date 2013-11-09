/*
 * application.h
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "kl_lib_L15x.h"

// Uart Command Codes. See https://docs.google.com/document/d/1pGQf9CrQ016ObS0w7PhPLAy92MRPhdBriICflt1YGXA/edit
#define CMD_PING            0x01
#define CMD_SET_GATE_NUM    0x10
#define CMD_RTX             0x11
#define CMD_PILL_STATE      0x30
#define CMD_PILL_WRITE      0x31
#define CMD_PILL_READ       0x32
#define CMD_PIN             0x40

#define RPL_ACK             0x90    // Acknowledge
#define RPL_SET_GATE_NUM    0xA0
#define RPL_RTX             0xA1    // TX completed
#define RPL_RRX             0xA4    // RX completed
#define RPL_PILL_STATE      0xC0
#define RPL_PILL_WRITE      0xC1
#define RPL_PILL_READ       0xC2
#define RPL_PIN             0xD0


class App_t {
private:

public:
    uint16_t ID;
    void Init();
};

extern App_t App;

#endif /* APPLICATION_H_ */
