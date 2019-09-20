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
#include "color.h"
#include "uart.h"
#include "MsgQ.h"

#if 1 // =========================== Pkt_t =====================================
struct rPkt_t {
    uint8_t r, g, b;
} __attribute__ ((__packed__));
#endif

#define RPKT_LEN                sizeof(rPkt_t)

// Message queue
#define R_MSGQ_LEN      4 // Length of q
#define R_MSG_SET_PWR   1
#define R_MSG_SET_CHNL  2
#define R_MSG_SEND_KILL 4
struct RMsg_t {
    uint8_t Cmd;
    uint8_t Value;
    RMsg_t() : Cmd(0), Value(0) {}
    RMsg_t(uint8_t ACmd) : Cmd(ACmd), Value(0) {}
    RMsg_t(uint8_t ACmd, uint8_t AValue) : Cmd(ACmd), Value(AValue) {}
} __attribute__((packed));

#if 1 // =================== Channels, cycles, Rssi  ===========================
#define RCHNL_SERVICE   0
#define RCHNL_COMMON    1
#define RCHNL_EACH_OTH  7
#define RCHNL_MIN       10
#define RCHNL_MAX       30
#define ID2RCHNL(ID)    (RCHNL_MIN + ID)

#define RSSI_MIN        -75

#define RSSI_FOR_MUTANT -120

// Feel-Each-Other related
#define CYCLE_CNT           4
#define SLOT_CNT            30
#define SLOT_DURATION_MS    5

// Timings
#define RX_T_MS                 180      // pkt duration at 10k is around 12 ms
#define RX_SLEEP_T_MS           810
#define MIN_SLEEP_DURATION_MS   18
#endif

class ColorAccumulator {
public:
	Color_t get() const { return Color_t(r, g, b); }
	void Add(rPkt_t packet) {
		r += packet.r;
		if (r > 255) r = 255;

		g += packet.g;
		if (g > 255) g = 255;

		b += packet.b;
		if (b > 255) b = 255;
	}

	void Decay(uint8_t step) {
		r = r > step ? r - step : 0;
		g = g > step ? g - step : 0;
		b = b > step ? b - step : 0;
	}

private:
	uint16_t r, g, b;
};

class rLevel1_t {
public:
    EvtMsgQ_t<RMsg_t, R_MSGQ_LEN> RMsgQ;
    ColorAccumulator accumulator;
    uint8_t Init();
    // Inner use
    void TryToSleep(uint32_t SleepDuration);
    void TryToReceive(uint32_t RxDuration);
    // Different modes of operation
    void TaskTransmitter();
    void TaskReceiverManyByID();
    void TaskReceiverManyByChannel();
    void TaskReceiverSingle();
    void TaskFeelEachOtherSingle();
    void TaskFeelEachOtherMany();
};

extern rLevel1_t Radio;
