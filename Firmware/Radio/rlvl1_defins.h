/*
 * rlvl1_defins.h
 *
 *  Created on: Nov 21, 2013
 *      Author: kreyl
 */

#ifndef RLVL1_DEFINS_H_
#define RLVL1_DEFINS_H_

#include "peripheral.h"
#include "payload.h"

#define MESH

#ifdef MESH
#include "mesh_params.h"

#if 1 // ============================== Pkt_t ========================================
struct rPkt_t {
    union {
        MeshPayload_t MeshPayload;
        struct {
            uint8_t SelfID;
            uint32_t CycleN;
            uint8_t TimeOwnerID;
            uint8_t TimeAge;
        }__attribute__ ((__packed__));
    }__attribute__ ((__packed__));
    // Payload
    uint16_t ID;
    union {
        PayloadString_t PktPayload;
        struct {
            uint8_t Hops;
            uint32_t TimeStamp;
            uint32_t TimeDiff;
            uint8_t Location;
            uint8_t Reason;
            uint8_t Emotion;
        } __attribute__ ((__packed__));
    } __attribute__ ((__packed__));
} __attribute__ ((__packed__));
#define RPKT_SZ     sizeof(rPkt_t)
#endif
#endif

#ifndef MESH
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
#endif



// Signal levels
#define RSSI_MIN_DB     (-110)
#define RSSI_MAX_DB     (-35)

#define RSSI_DB2PERCENT(db) ((((db) - RSSI_MIN_DB) * 100) / (RSSI_MAX_DB - RSSI_MIN_DB))

// Emanators
#define CHANNEL_ZERO        0
#define SLOW_EMANATOR_CNT   1
#define FAST_EMANATOR_CNT   0

#if 1 // =========================== Timings ===================================
#define RX_DURATION_SLOW_MS 270  // How long to listen


#endif



#endif /* RLVL1_DEFINS_H_ */
