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

/*********************** MESH ******************************
 *  |_|_|_|_..._|_|_|_|_|_..._|_|_|_|_|_..._|_|   SLOTS
 *  |_____________|_______...___|_____________|   CYCLES
 *  |_____________________..._________________|   SUPER_CYCLE
 */

#define SELF_MESH_ID        1

#define TABLE_SEND_N        3     /* send SnsTable after n cycles */
#define MAX_ABONENTS        100   /* max ID, started from 1 */

#define MESH_CHANNEL        1     /* mesh RF channel */
#define SLOT_TIME           4     /* ms */

#define COUNT_OF_CYCLES     5     /* count of cycles in supercycle */
#define CYCLE_TIME          (uint32_t)(SLOT_TIME * MAX_ABONENTS)
#define S_CYCLE_TIME        (uint32_t)(CYCLE_TIME * COUNT_OF_CYCLES)


//#define GET_RND_VALUE(Top)  ( ( (Random(chTimeNow()) ) % Top ))
#define GET_RND_VALUE(Top)    ( ((rand() % Top) + 1) )
//#define END_OF_EPOCH        4294967295 // ms = 2^32
#define END_OF_EPOCH        65536       // max cycle counter


#define TIME_AGE_THRESHOLD  20 /* Cycles */


#if 1 // ======================== Circ Buf of Pkt ==============================
#define CIRC_PKT_BUF_SZ     5 // MAX_ABONENTS FIXME: 5 size set to debug only

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
    uint8_t GetFreeSlots()          { return (uint8_t)(MAX_ABONENTS-FilledCount); }
    void WritePkt(mshMsg_t Ptr)     { *PWPkt++ = Ptr; FilledCount++; if(PWPkt >= (PktBuf + CIRC_PKT_BUF_SZ)) PWPkt = PktBuf; }
    void ReadPkt(mshMsg_t *Ptr)     { *Ptr = *PRPkt++; if(PRPkt >= (PktBuf + CIRC_PKT_BUF_SZ)) PRPkt = PktBuf; FilledCount--;}

};


#endif


#if 1// ============================== Mesh Class =============================
#define MESH_TIM                TIM7
#define MESH_TIM_IRQ            TIM7_IRQn
#define MESH_TIM_IRQ_HANDLER    TIM7_IRQHandler
#define RND_TBL_BUFFER_SZ       50

class Mesh_t {
private:
    uint8_t RndTableBuf[RND_TBL_BUFFER_SZ];
    uint8_t *PRndTable;
    uint32_t AbsCycle;
    uint32_t CurrCycle;
    uint32_t RxCycleN;
    uint32_t SleepTime;
    bool NeedUpdateTime;
    uint32_t SelfID;
    uint8_t NeedToSendTable;

    Timer_t CycleTmr;
    void NewRxCycle()       { RxCycleN = *PRndTable; PRndTable++; if(PRndTable > RndTableBuf + RND_TBL_BUFFER_SZ) PRndTable = RndTableBuf;   }
    void IncCurrCycle()     { AbsCycle++; CurrCycle++; if(CurrCycle >= COUNT_OF_CYCLES) { CurrCycle = 0; NewRxCycle(); } }

    void NewCycle();
    void TableSend();
    void UpdateTimer(bool NeedUpdate, uint32_t NewTime, uint32_t WakeUpSysTime);
    bool DispatchPkt(uint32_t *PTime, uint32_t *PWakeUpSysTime);
    void ResetTimeAge(uint8_t ID)     { Radio.ResetTimeAge(ID); }
    uint8_t GetTimeAge()              { return Radio.GetTimeAge(); }
    uint8_t GetMeshID()               { return Radio.GetTimeOwner(); }

public:
    Mesh_t() :  PRndTable(RndTableBuf),
                AbsCycle(0),
                CurrCycle(COUNT_OF_CYCLES),
                RxCycleN(*PRndTable),
                SleepTime(0),
                NeedUpdateTime(false),
                SelfID(0),
                NeedToSendTable(0),
                IPThread(NULL),
                LedColor(clCyan),
                INeedColor(clBlack) {}

    Thread *IPThread;
    CircBufPkt_t PktBuf;
    uint32_t GetCycleN()                { return (AbsCycle);             }
    uint32_t GetAbsTimeMS()             { return (AbsCycle*CYCLE_TIME);  }
//    void SetAbsTimeMS(uint32_t MS)      { AbsCycle = (MS + (CYCLE_TIME/2)) / CYCLE_TIME; }
    void SetCurrCycleN(uint32_t ANew)   { AbsCycle = ANew; CurrCycle = 0; NewRxCycle(); }
    MsgBox_t<mshMsg_t, RPKT_SZ> MsgBox;
    void Init(uint32_t ID);

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
};

extern Mesh_t Mesh;
#endif

#endif /* MESH_LVL_H_ */
