/*
 * radio_lvl1.h
 *
 *  Created on: Nov 17, 2013
 *      Author: kreyl
 */

#ifndef RADIO_LVL1_H_
#define RADIO_LVL1_H_

#include "ch.h"

#if 1 // =========================== Pkt_t =====================================
struct rPkt_t {
    uint8_t MinLvl;
    uint8_t MaxLvl;
    uint8_t ConstDmg;
    uint8_t VarDmgMin;
    uint8_t VarDmgMax;
    int8_t RSSI;        // Received signal level, RX use only
} __attribute__ ((__packed__));
#define RPKT_LEN    (sizeof(rPkt_t)-1)  // Exclude RSSI
#endif

// Emanators
#define CHANNEL_ZERO        10
#define SLOW_EMANATOR_CNT   1
#define FAST_EMANATOR_CNT   0

#if 1 // =========================== Timings ===================================
#define RX_DURATION_SLOW_MS 99  // How long to listen


#endif

class rLevel1_t {
private:
    rPkt_t PktRx;       // Local rPkt to receive
    rPkt_t PktTx;
public:

    void Init();
    // Inner use
    void ITask();
};

extern rLevel1_t Radio;

#endif /* RADIO_LVL1_H_ */
