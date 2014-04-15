/*
 * mesh_lvl.cpp
 *
 *  Created on: 28 февр. 2014 г.
 *      Author: r.leonov
 */


#include "mesh_lvl.h"
#include "radio_lvl1.h"
#include "peripheral.h"
#include "sequences.h"
#include "application.h"
#include "real_time.h"

//#define MESH_DBG        // On/Off Debug Message in MeshLvl

//pyton translation for db
//[22:19:36] Jolaf: str(tuple(1 + int(sqrt(float(i) / 65) * 99) for i in xrange(0, 65 + 1)))
const int DbTranslate[66] = {1, 13, 18, 22, 25, 28, 31, 33, 35, 37, 39, 41, 43, 45, 46, 48, 50, 51, 53, 54, 55, 57, 58, 59, 61, 62, 63, 64, 65, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 86, 87, 88, 89, 90, 91, 92, 92, 93, 94, 95, 96, 96, 97, 98, 99, 100};

Mesh_t Mesh;

// ================================= Thread ====================================
extern "C" {
CH_IRQ_HANDLER(MESH_TIM_IRQ_HANDLER) {
    CH_IRQ_PROLOGUE();
    chSysLockFromIsr();
    Mesh.IIrqHandler();
    chSysUnlockFromIsr();
    CH_IRQ_EPILOGUE();
}
}

static WORKING_AREA(waMeshLvlThread, 256);
__attribute__ ((__noreturn__))
static void MeshLvlThread(void *arg) {
    chRegSetThreadName("MeshLvl");
    while(1) Mesh.ITask();
}

void Mesh_t::ITask() {
    uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);

    if(EvtMsk & EVTMSK_NEW_CYCLE) NewCycle();

    if(EvtMsk & EVTMSK_UPDATE_CYCLE) {
        uint32_t NewAbsTime=0;
        uint32_t NextCycleStart=0;
        NeedUpdateTime = DispatchPkt(&NewAbsTime, &NextCycleStart);
//        TableSend();
        UpdateTimer(NeedUpdateTime, NewAbsTime, NextCycleStart);
    }
}

void Mesh_t::NewCycle() {
//    Uart.Printf("i,%u, t=%u\r", AbsCycle, chTimeNow());
//    Beeper.Beep(ShortBeep);
    IncCurrCycle();
    // ==== RX ====
    if(CurrCycle == RxCycleN) {
        Radio.SendEvt(EVTMSK_MESH_RX);
        mshMsg_t MeshPkt;
        do {
            if(MsgBox.TryFetchMsg(&MeshPkt) == OK) PktBucket.WritePkt(MeshPkt);
        } while(Radio.IMeshRx);
    }
    // ==== TX ====
    else {
        if(SleepTime > 0) chThdSleepMilliseconds(SleepTime);
        Radio.SendEvt(EVTMSK_MESH_TX);
    }
}

bool Mesh_t::DispatchPkt(uint32_t *PTime, uint32_t *PWakeUpSysTime) {
    bool GetPrimaryPkt = false;


    if(PktBucket.GetFilledSlots() != 0) {
        uint8_t PriorityID = GetMeshID();
        mshMsg_t MeshMsg;
        do {
            PktBucket.ReadPkt(&MeshMsg);
            Uart.Printf("Msh: ID=%u, CycleN=%u, TimOwnID=%u\r", MeshMsg.MeshPayload.SelfID, MeshMsg.MeshPayload.CycleN, MeshMsg.MeshPayload.TimeOwnerID);
            if(PriorityID > MeshMsg.MeshPayload.TimeOwnerID) GetPrimaryPkt = true;
            else if(PriorityID == MeshMsg.MeshPayload.TimeOwnerID) {
                if(GetTimeAge() > MeshMsg.MeshPayload.TimeAge) {  /* compare TimeAge */
                    GetPrimaryPkt = true;                   /* need to update */
                }
            }

            if(GetPrimaryPkt) {
                CycleTmr.Disable();
                *PTime = MeshMsg.MeshPayload.CycleN + 1;
                PriorityID = MeshMsg.MeshPayload.TimeOwnerID;
                ResetTimeAge(PriorityID);
                *PWakeUpSysTime = MeshMsg.Timestamp + (uint32_t)CYCLE_TIME - (SLOT_TIME * PriorityID);
            }
//            if(MeshMsg.RSSI < -100) MeshMsg.RSSI = -100;
//            else if(MeshMsg.RSSI > -35) MeshMsg.RSSI = -35;
//            MeshMsg.RSSI += 100;    // 0...65
//            uint32_t Lvl = DbTranslate[MeshMsg.RSSI]; //1 + (uint32_t)(((int32_t)MeshMsg.RSSI * 99) / 65);
//            SnsTable.PutSnsInfo(MeshMsg.PktRx.ID, Lvl);   /* Put Information in SensTable */
        } while(PktBucket.GetFilledSlots() != 0);
    }
    return GetPrimaryPkt;
}

void Mesh_t::UpdateTimer(bool NeedUpdate, uint32_t NewTime, uint32_t WakeUpSysTime) {
    if(NeedUpdateTime) {
#ifdef MESH_DBG
        Uart.Printf("Msh: CycUpd=%u\r", NewTime);
#endif
        uint32_t timeNow = chTimeNow();
        do {
            WakeUpSysTime += CYCLE_TIME;
//            NewTime++;  /* TODO: Thinking carefully about updating TimeAbs */
        }
        while (WakeUpSysTime < timeNow);
        SetCurrCycleN(NewTime);
        CycleTmr.SetCounter(0);
        NeedUpdateTime = false;
#ifdef MESH_DBG
        Uart.Printf("Msh: Slp %u to %u\r", chTimeNow(), WakeUpSysTime);
#endif
        chThdSleepUntil(WakeUpSysTime); /* TODO: Thinking carefully about asynch switch on Timer with Virtual timer */
        CycleTmr.Enable();
    }
}

void Mesh_t::Init(uint32_t ID) {
    if(ID == 0) {
        Uart.Printf("Msh: WrongID\r");
        return;
    }
    // Init Thread
    IPThread = chThdCreateStatic(waMeshLvlThread, sizeof(waMeshLvlThread), NORMALPRIO, (tfunc_t)MeshLvlThread, NULL);
    SelfID = (uint8_t)ID;
    SleepTime = ((SelfID-1)*SLOT_TIME);
    MsgBox.Init();
    GenerateRandomTable(RND_TBL_BUFFER_SZ);

    CycleTmr.Init(MESH_TIM);
    CycleTmr.SetupPrescaler(1000);
    CycleTmr.SetTopValue(CYCLE_TIME-1);
    CycleTmr.SetCounter(0);
    CycleTmr.IrqOnTriggerEnable();
    CycleTmr.Enable();
    nvicEnableVector(MESH_TIM_IRQ, CORTEX_PRIORITY_MASK(IRQ_PRIO_HIGH));
    Uart.Printf("Msh: Init ID=%u\r", SelfID);
//    Led.SetColor(LedColor);
}

void Mesh_t::IIrqHandler() {
    CycleTmr.ClearIrqPendingBit();
    if(IPThread != nullptr) chEvtSignalI(IPThread, EVTMSK_NEW_CYCLE);
}
