/*
1616 * mesh_params.h
 *
 *  Created on: 15 апр. 2014 г.
 *      Author: r.leonov
 */

#ifndef MESH_PARAMS_H_
#define MESH_PARAMS_H_

/*********************** MESH ******************************
 *  |_|_|_|_..._|_|_|_|_|_..._|_|_|_|_|_..._|_|   SLOTS
 *  |_____________|_______...___|_____________|   CYCLES
 *  |_____________________..._________________|   SUPER_CYCLE
 */

//#define SELF_MESH_ID        1
#define STATIONARY_ID           6
#define STATIONARY_MIN_LEVEL    -120

// ==== MESH PARAMS ====
#define MAX_ABONENTS        100   /* max ID, started from 1 */
#define MESH_CHANNEL        1     /* mesh RF channel */
#define MESH_PKT_TIME       3
#define MESH_GUARD_TIME     7

/* SLOT_TIME is equivalent to doubled time of transmit rf packet */
#define SLOT_TIME           (uint32_t)(MESH_PKT_TIME + MESH_GUARD_TIME)    /* 3 ms normal time for transmit packet, take it as 10 ms */

#define MESH_COUNT_OF_CYCLES     5     /* count of cycles in supercycle */

#define CYCLE_TIME          (uint32_t)((SLOT_TIME * MAX_ABONENTS))
#define S_CYCLE_TIME        (uint32_t)(CYCLE_TIME * COUNT_OF_CYCLES)


//#define GET_RND_VALUE(Top)  ( ( (Random(chTimeNow()) ) % Top ))
#define GET_RND_VALUE(Top)    (rand() % (Top))


#define TIME_AGE_THRESHOLD  20 /* Cycles */
#define START_CYCLE         1


struct meshradio_t {
    VirtualTimer RxVT;
    bool InRx;
    uint32_t RxEndTime;
    uint32_t CurrentTime;
};

struct MeshPayload_t {
    uint16_t SelfID;
    uint32_t CycleN;
    uint16_t TimeOwnerID;
    uint8_t TimeAge;
    uint8_t SelfLocation;
    uint8_t SelfReason;
    uint8_t SelfEmotion;
} __attribute__ ((__packed__));

struct PayloadString_t {
    uint8_t Hops;
    uint32_t Timestamp;
    int32_t TimeDiff;
    uint8_t Location;
    uint8_t Reason;
    uint8_t Emotion;
} __attribute__ ((__packed__));

struct MeshPkt_t {
    MeshPayload_t MeshData;
    uint16_t PayloadID;
    PayloadString_t Payload;
}__attribute__ ((__packed__));

#define MESH_PKT_SZ sizeof(MeshPkt_t)

#endif /* MESH_PARAMS_H_ */
