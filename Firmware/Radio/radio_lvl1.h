/*
 * radio_lvl1.h
 *
 *  Created on: Nov 17, 2013
 *      Author: kreyl
 */

#pragma once

#include "kl_lib.h"
#include "ch.h"
#include "cc1101.h"
#include "kl_buf.h"
#include "uart.h"
#include "color.h"

#if 1 // =========================== Pkt_t =====================================

enum RCmd_t {
    rcmdNone = 0,
    rcmdPing = 1,
    rcmdPong = 2,
    rcmdBeacon = 3,
    rcmdScream = 4,
    rcmdLustraParams = 5,
    rcmdLocketParams = 6,
    rcmdLocketExplode = 7,
    rcmdLocketDie = 8,
};

struct rPkt_t {
    uint16_t From;
    uint16_t To;
    uint16_t TransmitterID;
    RCmd_t Cmd : 4;
    uint8_t PktID : 4; // Do not retransmit if zero
    union {
        struct {
            uint16_t MaxLvlID;
            uint8_t Reply;
        } Pong; // 3

        struct {
            int8_t RssiThr;
            uint8_t Damage;
        } Beacon; // 2

        struct {
            uint8_t Power;
            int8_t RssiThr;
            uint8_t Damage;
        } LustraParams; // 3

        struct {
            uint8_t MaxHP;
        } LocketParams; // 1

        struct {
            int8_t RssiThr;
        } Die; // 1
    }; // union
} __attribute__ ((__packed__));

#define PKTID_DO_NOT_RETRANSMIT 0
#define PKTID_TOP_VALUE         0x0F
#endif


#if 1 // ============================== Defines ================================
#define RCHNL                   0
#define RPKT_LEN                sizeof(rPkt_t)
#define CC_TX_PWR               CC_Pwr0dBm

#define CMD_GET                 18
#define CMD_SET                 27
#define CMD_FLASH               36
#define CMD_OK                  0

#define RX_T_MS                 99
#define RX_SLEEP_T_MS           810

#endif

class rLevel1_t {
public:
    uint8_t Init();
    rPkt_t PktRx;
    // Inner use
    void TryToSleep(uint32_t SleepDuration);
    void TryToReceive(uint32_t RxDuration);
};

extern rLevel1_t Radio;
