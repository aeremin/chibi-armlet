/*
 * mesh_lvl.h
 *
 *  Created on: 28 февр. 2014 г.
 *      Author: r.leonov
 */

#ifndef MESH_LVL_H_
#define MESH_LVL_H_

#include "kl_lib_L15x.h"
#include "evt_mask.h"
#include "radio_lvl1.h"
#include "msg_box.h"
#include "stdlib.h"

#include "mesh_params.h"
#include "payload.h"
#include "application.h"

#if 1 // ======================== Circ Buf of Pkt ==============================
#define CIRC_PKT_BUF_SZ     5 // MAX_ABONENTS FIXME: 5 size set to debug only

struct CircBufString_t {
    uint32_t Timestamp;
    int8_t RSSI;
    MeshPkt_t RadioPkt;
};

class CircBufPkt_t {
private:
    CircBufString_t PktBuf[CIRC_PKT_BUF_SZ];
    CircBufString_t *PRPkt;
    CircBufString_t *PWPkt;
    uint8_t FilledCount;
public:
    CircBufPkt_t() :
        PRPkt(PktBuf), PWPkt(PktBuf),
        FilledCount(0) {}

    uint8_t GetFilledSlots()        { return (uint8_t)FilledCount; }
    uint8_t GetFreeSlots()          { return (uint8_t)(CIRC_PKT_BUF_SZ-FilledCount); }
    void WritePkt(mshMsg_t Ptr)     {
        PWPkt->Timestamp =          Ptr.Timestamp;
        PWPkt->RSSI =               Ptr.RSSI;
        PWPkt->RadioPkt.MeshData =  Ptr.pRadioPkt->MeshData;
        PWPkt->RadioPkt.PayloadID = Ptr.pRadioPkt->PayloadID;
        PWPkt->RadioPkt.Payload =   Ptr.pRadioPkt->Payload;
        PWPkt++;
        FilledCount++;
        if(PWPkt >= (PktBuf + CIRC_PKT_BUF_SZ)) PWPkt = PktBuf;
    }
    void ReadPkt(CircBufString_t *Ptr)     {
        *Ptr = *PRPkt++;
        if(PRPkt >= (PktBuf + CIRC_PKT_BUF_SZ)) PRPkt = PktBuf;
        FilledCount--;
    }
};
#endif

#if 1// ============================== Mesh Class =============================
#define MESH_TIM                TIM7
#define MESH_TIM_IRQ            TIM7_IRQn
#define MESH_TIM_IRQ_HANDLER    TIM7_IRQHandler

#define RND_TBL_BUFFER_SZ       25

class Mesh_t {
private:
    uint8_t RndTableBuf[RND_TBL_BUFFER_SZ];
    uint8_t *PRndTable;

    uint32_t AbsCycle;
    uint32_t CurrCycle;
    uint32_t RxCycleN;
    uint32_t SleepTime;
    int8_t PreliminaryRSSI;
//    uint16_t SelfID;
//    uint8_t NeedToSendTable;

    // Synchronization
    bool GetPrimaryPkt;
    uint16_t PriorityID;
    uint32_t NewCycleN, *PNewCycleN;
    uint32_t TimeToWakeUp, *PTimeToWakeUp;

    Timer_t CycleTmr;
    CircBufString_t MeshMsg;

    void INewRxCycle()       {
        PRndTable++;
        if(PRndTable > RndTableBuf + RND_TBL_BUFFER_SZ) PRndTable = RndTableBuf;
        RxCycleN = *PRndTable;
    }

    void IIncCurrCycle()     {
        AbsCycle++;
        CurrCycle++;
        if(CurrCycle >= MESH_COUNT_OF_CYCLES) {
            CurrCycle = 0;
            INewRxCycle();
        }
    }
    void IGenerateRandomTable(uint32_t Size) {
        srand(App.ID);
        RndTableBuf[0] = 1;
        for(uint8_t i=1; i<RND_TBL_BUFFER_SZ; i++) {
            RndTableBuf[i] = GET_RND_VALUE(MESH_COUNT_OF_CYCLES);
        }
        RxCycleN = *PRndTable;
        // TODO: check random table value
    }

    void INewCycle();
    void IUpdateTimer();
    void IPktPutCycle(uint32_t NewCycle)            { PktTx.MeshData.CycleN = NewCycle; }
    void IPktPutTimeOwner(uint16_t NewTimeOwner)    { PktTx.MeshData.TimeOwnerID = NewTimeOwner; }
    uint16_t IGetTimeOwner()                        { return PktTx.MeshData.TimeOwnerID; }
    void ITimeAgeCounter() {
        if(PktTx.MeshData.SelfID != PktTx.MeshData.TimeOwnerID) {
            PktTx.MeshData.TimeAge++;
            if(PktTx.MeshData.TimeAge > TIME_AGE_THRESHOLD) IResetTimeAge(App.ID, 0);
        }
    }
    void IResetTimeAge(uint16_t NewID, uint8_t TA)  { PktTx.MeshData.TimeAge = TA; PktTx.MeshData.TimeOwnerID = NewID; }
    uint8_t IGetTimeAge()                           { return PktTx.MeshData.TimeAge; }

public:
    Mesh_t() :
                PRndTable(RndTableBuf),
                AbsCycle(START_CYCLE),
                CurrCycle(0),
                RxCycleN(*PRndTable),
                SleepTime(0),
//                NeedUpdateTime(false),
//                SelfID(0),
                PreliminaryRSSI(STATIONARY_MIN_LEVEL),
                GetPrimaryPkt(false),
                PriorityID(0),
                NewCycleN(0),
                PNewCycleN(&NewCycleN),
                TimeToWakeUp(0),
                PTimeToWakeUp(&TimeToWakeUp),
//                NeedToSendTable(0),
                IPThread(NULL),
                IPPktHanlderThread(NULL),
                IsInit(false)  {}

    Thread *IPThread, *IPPktHanlderThread;
    bool IsInit;

//    void NewSelfID(uint32_t NewSelfID)  { SelfID = NewSelfID; }
    void UpdateSleepTime()              { SleepTime = ((App.ID-1)*SLOT_TIME); }
    uint32_t GetCycleN()                { return (AbsCycle);             }
    uint32_t GetAbsTimeMS()             { return (AbsCycle*CYCLE_TIME);  }
//    void SetAbsTimeMS(uint32_t MS)      { AbsCycle = (MS + (CYCLE_TIME/2)) / CYCLE_TIME; }
    int32_t SetNewAbsCycleN(uint32_t ANew)   {
        int32_t Diff = AbsCycle;
        AbsCycle = ANew;
        CurrCycle = AbsCycle % MESH_COUNT_OF_CYCLES;
        INewRxCycle();
        Diff = AbsCycle - Diff;
        return Diff;
    }
    void Init();

    MeshPkt_t PktRx, PktTx;
    MsgBox_t<mshMsg_t, MESH_PKT_SZ> MsgBox;
    CircBufPkt_t PktBucket;

    void ITask();
    void IWaitTxEnd();
    void ITxEnd();
    void IIrqHandler();
    void IPktHandler();
    void SendEvent(eventmask_t mask)  { chEvtSignal(IPThread,mask); }
    void PreparePktPayload();
};

extern Mesh_t Mesh;


#endif

#endif /* MESH_LVL_H_ */
