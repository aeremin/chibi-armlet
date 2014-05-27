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
#define STATIONARY_ID       10


#define MAX_ABONENTS        100   /* max ID, started from 1 */

#define MESH_CHANNEL        1     /* mesh RF channel */

/* SLOT_TIME is equivalent to doubled time of transmit rf packet */
#define SLOT_TIME           30    /* 26 ms normal time for transmit packet, take it as 30 ms */

#define COUNT_OF_CYCLES     5     /* count of cycles in supercycle */

#define CYCLE_TIME          (uint32_t)((SLOT_TIME * MAX_ABONENTS))
#define S_CYCLE_TIME        (uint32_t)(CYCLE_TIME * COUNT_OF_CYCLES)


//#define GET_RND_VALUE(Top)  ( ( (Random(chTimeNow()) ) % Top ))
#define GET_RND_VALUE(Top)    ( ((rand() % Top) + 1) )


#define TIME_AGE_THRESHOLD  20 /* Cycles */

struct meshradio_t {
    VirtualTimer RxVT;
    uint16_t RxTmt;
    bool InRx;
    uint32_t CurrentTime;
};

struct MeshPayload_t {
    uint16_t SelfID;
    uint32_t CycleN;
    uint16_t TimeOwnerID;
    uint8_t TimeAge;
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
