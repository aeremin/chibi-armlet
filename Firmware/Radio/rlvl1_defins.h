/*
 * rlvl1_defins.h
 *
 *  Created on: Nov 21, 2013
 *      Author: kreyl
 */

#ifndef RLVL1_DEFINS_H_
#define RLVL1_DEFINS_H_

// ==== pyton translation for db ====
#define RX_LVL_TOP      1000
// Jolaf: str(tuple(1 + int(sqrt(float(i) / 65) * 99) for i in xrange(0, 65 + 1)))
const int32_t dBm2Percent1000Tbl[66] = {10, 130, 180, 220, 250, 280, 310, 330, 350, 370, 390, 410, 430, 450, 460, 480, 500, 510, 530, 540, 550, 570, 580, 590, 610, 620, 630, 640, 650, 670, 680, 690, 700, 710, 720, 730, 740, 750, 760, 770, 780, 790, 800, 810, 820, 830, 840, 850, 860, 860, 870, 880, 890, 900, 910, 920, 920, 930, 940, 950, 960, 960, 970, 980, 990, 1000};


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
