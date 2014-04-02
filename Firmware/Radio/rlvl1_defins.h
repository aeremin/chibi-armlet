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
    uint8_t Type;
    uint8_t Check[3];
} __attribute__ ((__packed__));
#define RPKT_LEN    sizeof(rPkt_t)
#define CHECK_0         0xA5
#define CHECK_1         0xB4
#define CHECK_2         0xF2
#endif

#if 1 // ======================= Address space =================================
#define RCHNL_MIN       10
#define RCHNL_MAX       60


#endif

// Signal levels
#define RSSI_MIN_DB     (-110)
#define RSSI_MAX_DB     (-35)

#define RSSI_DB2PERCENT(db) ((((db) - RSSI_MIN_DB) * 100) / (RSSI_MAX_DB - RSSI_MIN_DB))


#define FIELD_RX_CHNL       9
#define DETECTOR_TX_CNT     4

#if 1 // =========================== Timings ===================================
#define RX_TIMEOUT_MS       4

#endif



#endif /* RLVL1_DEFINS_H_ */
