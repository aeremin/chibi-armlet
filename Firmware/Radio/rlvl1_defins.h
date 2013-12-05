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
    uint8_t ID;
    uint8_t MinLvl;
    uint8_t MaxLvl;
    uint8_t DmgMin;
    uint8_t DmgMax;
    int8_t RSSI;        // Received signal level, RX use only
} __attribute__ ((__packed__));
#define RPKT_LEN    (sizeof(rPkt_t)-1)  // Exclude RSSI
#endif

// Signal levels
#define RSSI_MIN_DB     (-110)
#define RSSI_MAX_DB     (-35)

#define RSSI_DB2PERCENT(db) ((((db) - RSSI_MIN_DB) * 100) / (RSSI_MAX_DB - RSSI_MIN_DB))

static inline int32_t GetRadioDmg(rPkt_t *PPkt) {
    int32_t EmDmg = 0;
    int32_t prc = RSSI_DB2PERCENT(PPkt->RSSI);
    if(prc >= PPkt->MaxLvl) EmDmg = PPkt->DmgMax;
    else if(prc >= PPkt->MinLvl) {
        int32_t DifDmg = PPkt->DmgMax - PPkt->DmgMin;
        int32_t DifLvl = PPkt->MaxLvl - PPkt->MinLvl;
        EmDmg = (prc * DifDmg + PPkt->DmgMax * DifLvl - PPkt->MaxLvl * DifDmg) / DifLvl;
        if(EmDmg < 0) EmDmg = 0;
    }
    return EmDmg;
}



// Emanators
#define CHANNEL_ZERO        0
#define SLOW_EMANATOR_CNT   1
#define FAST_EMANATOR_CNT   0

#if 1 // =========================== Timings ===================================
#define RX_DURATION_SLOW_MS 270  // How long to listen


#endif



#endif /* RLVL1_DEFINS_H_ */
