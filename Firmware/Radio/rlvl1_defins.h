/*
 * rlvl1_defins.h
 *
 *  Created on: Nov 21, 2013
 *      Author: kreyl
 */

#ifndef RLVL1_DEFINS_H_
#define RLVL1_DEFINS_H_

//pyton translation for db
//[22:19:36] Jolaf: str(tuple(1 + int(sqrt(float(i) / 65) * 99) for i in xrange(0, 65 + 1)))
const uint8_t dBm2PercentTbl[66] = {1, 13, 18, 22, 25, 28, 31, 33, 35, 37, 39, 41, 43, 45, 46, 48, 50, 51, 53, 54, 55, 57, 58, 59, 61, 62, 63, 64, 65, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 86, 87, 88, 89, 90, 91, 92, 92, 93, 94, 95, 96, 96, 97, 98, 99, 100};


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
#define FIELD_RX_CHNL    9
#define RCHNL_MIN       FIELD_RX_CHNL
#define RCHNL_MAX       60
#endif

// Signal levels
#define RSSI_MIN_DB     (-110)
#define RSSI_MAX_DB     (-35)

#define DETECTOR_TX_CNT     4

#if 1 // =========================== Timings ===================================
#define FIELD_RX_T_MS       4
#define RCVR_RX_T_MS        6

#endif



#endif /* RLVL1_DEFINS_H_ */
