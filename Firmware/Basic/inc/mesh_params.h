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

#define SELF_MESH_ID        2

#define TABLE_SEND_N        3     /* send SnsTable after n cycles */
#define MAX_ABONENTS        100   /* max ID, started from 1 */

#define MESH_CHANNEL        1     /* mesh RF channel */
#define SLOT_TIME           30    /* 26 ms normal time for transmitt packet, take it as 30 ms */

#define COUNT_OF_CYCLES     5     /* count of cycles in supercycle */
#define CYCLE_TIME          (uint32_t)(SLOT_TIME * MAX_ABONENTS)
#define S_CYCLE_TIME        (uint32_t)(CYCLE_TIME * COUNT_OF_CYCLES)


//#define GET_RND_VALUE(Top)  ( ( (Random(chTimeNow()) ) % Top ))
#define GET_RND_VALUE(Top)    ( ((rand() % Top) + 1) )
//#define END_OF_EPOCH        4294967295 // ms = 2^32
//#define END_OF_EPOCH        65536       // max cycle counter


#define TIME_AGE_THRESHOLD  20 /* Cycles */

struct MeshPayload_t {
    uint8_t SelfID;
    uint32_t CycleN;
    uint8_t TimeOwnerID;
    uint8_t TimeAge;
} __attribute__ ((__packed__));

#define MESH_PAYLOAD_SZ sizeof(MeshPayload_t)

#endif /* MESH_PARAMS_H_ */
