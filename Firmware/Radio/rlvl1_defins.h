/*
 * rlvl1_defins.h
 *
 *  Created on: Nov 21, 2013
 *      Author: kreyl
 */

#ifndef RLVL1_DEFINS_H_
#define RLVL1_DEFINS_H_

#if 1 // =========================== Pkt_t =====================================
struct rPkt_t {
    int8_t MinLvlDb;
    int8_t MaxLvlDb;
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
#define RX_DURATION_SLOW_MS 270  // How long to listen


#endif



#endif /* RLVL1_DEFINS_H_ */
