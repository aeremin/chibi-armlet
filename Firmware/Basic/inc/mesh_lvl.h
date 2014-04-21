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

#if 1 // ======================== Circ Buf of Pkt ==============================
#define CIRC_PKT_BUF_SZ     21 // MAX_ABONENTS FIXME: 5 size set to debug only

class CircBufPkt_t {
private:
    mshMsg_t PktBuf[CIRC_PKT_BUF_SZ];
    mshMsg_t *PRPkt;
    mshMsg_t *PWPkt;
    uint8_t FilledCount;
public:
    CircBufPkt_t() :
        PRPkt(PktBuf), PWPkt(PktBuf),
        FilledCount(0) {}

    uint8_t GetFilledSlots()        { return (uint8_t)FilledCount; }
    uint8_t GetFreeSlots()          { return (uint8_t)(CIRC_PKT_BUF_SZ-FilledCount); }
    void WritePkt(mshMsg_t Ptr)     { *PWPkt++ = Ptr; FilledCount++; if(PWPkt >= (PktBuf + CIRC_PKT_BUF_SZ)) PWPkt = PktBuf; }
    void ReadPkt(mshMsg_t *Ptr)     { *Ptr = *PRPkt++; if(PRPkt >= (PktBuf + CIRC_PKT_BUF_SZ)) PRPkt = PktBuf; FilledCount--;}
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
//    bool NeedUpdateTime;
    uint16_t SelfID;
//    uint8_t NeedToSendTable;

    // Synchronization
    bool GetPrimaryPkt;
    uint16_t PriorityID;
    uint32_t NewCycleN, *PNewCycleN;
    uint32_t TimeToWakeUp, *PTimeToWakeUp;

    Timer_t CycleTmr;
    CircBufPkt_t PktBucket;
    void NewRxCycle()       { RxCycleN = *PRndTable; PRndTable++; if(PRndTable > RndTableBuf + RND_TBL_BUFFER_SZ) PRndTable = RndTableBuf;   }
    void IncCurrCycle()     { AbsCycle++; CurrCycle++; if(CurrCycle >= COUNT_OF_CYCLES) { CurrCycle = 0; NewRxCycle(); } }
    void GenerateRandomTable(uint32_t Size) {
        for(uint8_t i=0; i<RND_TBL_BUFFER_SZ; i++) {
            RndTableBuf[i] = GET_RND_VALUE(COUNT_OF_CYCLES);
        }
    }

    void NewCycle();
    void UpdateTimer();
    void ResetTimeAge(uint8_t ID)     { Radio.ResetTimeAge(ID); }
    uint8_t GetTimeAge()              { return Radio.GetTimeAge(); }
    uint8_t GetMeshID()               { return Radio.GetTimeOwner(); }

    void PktHandlerStart() {
        chEvtSignal(IPBktHanlder, EVT_MSK_BKT_EXTRACT);
    }
public:
    Mesh_t() :  PRndTable(RndTableBuf),
                AbsCycle(0),
                CurrCycle(COUNT_OF_CYCLES),
                RxCycleN(*PRndTable),
                SleepTime(0),
//                NeedUpdateTime(false),
                SelfID(0),
                GetPrimaryPkt(false),
                PriorityID(0),
                NewCycleN(0),
                PNewCycleN(&NewCycleN),
                TimeToWakeUp(0),
                PTimeToWakeUp(&TimeToWakeUp),
//                NeedToSendTable(0),
                IPThread(NULL),
                IPBktHanlder(NULL),
                LedColor(clCyan),
                INeedColor(clBlack) {}

    Thread *IPThread, *IPBktHanlder;
    uint32_t GetCycleN()                { return (AbsCycle);             }
    uint32_t GetAbsTimeMS()             { return (AbsCycle*CYCLE_TIME);  }
//    void SetAbsTimeMS(uint32_t MS)      { AbsCycle = (MS + (CYCLE_TIME/2)) / CYCLE_TIME; }
    void SetCurrCycleN(uint32_t ANew)   { AbsCycle = ANew; CurrCycle = 0; NewRxCycle(); }
    void Init(uint32_t ID);
    MsgBox_t<mshMsg_t, MESH_PAYLOAD_SZ> MsgBox;
    Color_t LedColor, INeedColor;
    Color_t GetColor(uint8_t LedColor) {
        switch (LedColor) {
            case 1: return clBlack; break;
            case 2: return clRed; break;
            case 3: return clGreen; break;
            case 4: return clBlue; break;
            case 5: return clYellow; break;
            case 6: return clMagenta; break;
            case 7: return clCyan; break;
            case 8: return clWhite; break;
            default: return clBlack;
        }
        return clBlack;
    }


    void ITask();
    void IIrqHandler();
    void IPktHandler();
    void SendEvent(eventmask_t mask)  { chEvtSignal(IPThread,mask); }
};

extern Mesh_t Mesh;
#endif

#endif /* MESH_LVL_H_ */
