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
    bool NeedUpdateTime;
    uint16_t SelfID;
//    uint8_t NeedToSendTable;

    Timer_t CycleTmr;
    void NewRxCycle()       { RxCycleN = *PRndTable; PRndTable++; if(PRndTable > RndTableBuf + RND_TBL_BUFFER_SZ) PRndTable = RndTableBuf;   }
    void IncCurrCycle()     { AbsCycle++; CurrCycle++; if(CurrCycle >= COUNT_OF_CYCLES) { CurrCycle = 0; NewRxCycle(); } }
    void GenerateRandomTable(uint32_t Size) {
        for(uint8_t i=0; i<RND_TBL_BUFFER_SZ; i++) {
            RndTableBuf[i] = GET_RND_VALUE(COUNT_OF_CYCLES);
        }
    }

    void NewCycle();
//    void TableSend();
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
//                NeedToSendTable(0),
                IPThread(NULL),
                LedColor(clCyan),
                INeedColor(clBlack) {}

    Thread *IPThread;
    uint32_t GetCycleN()                { return (AbsCycle);             }
    uint32_t GetAbsTimeMS()             { return (AbsCycle*CYCLE_TIME);  }
//    void SetAbsTimeMS(uint32_t MS)      { AbsCycle = (MS + (CYCLE_TIME/2)) / CYCLE_TIME; }
    void SetCurrCycleN(uint32_t ANew)   { AbsCycle = ANew; CurrCycle = 0; NewRxCycle(); }
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
    void SendEvent(eventmask_t mask)  { chEvtSignal(IPThread,mask); }
};

extern Mesh_t Mesh;
#endif

#endif /* MESH_LVL_H_ */
