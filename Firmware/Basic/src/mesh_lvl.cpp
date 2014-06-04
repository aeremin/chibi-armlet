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

//#define MESH_DBG        // On/Off Debug Message in MeshLvl

//pyton translation for db
//[22:19:36] Jolaf: str(tuple(1 + int(sqrt(float(i) / 65) * 99) for i in xrange(0, 65 + 1)))
const int DbTranslate[66] = {1, 13, 18, 22, 25, 28, 31, 33, 35, 37, 39, 41, 43, 45, 46, 48, 50, 51, 53, 54, 55, 57, 58, 59, 61, 62, 63, 64, 65, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 86, 87, 88, 89, 90, 91, 92, 92, 93, 94, 95, 96, 96, 97, 98, 99, 100};

Mesh_t Mesh;

#if 1 // ================================= Thread ====================================
extern "C" {
CH_IRQ_HANDLER(MESH_TIM_IRQ_HANDLER) {
    CH_IRQ_PROLOGUE();
    chSysLockFromIsr();
    Mesh.IIrqHandler();
    chSysUnlockFromIsr();
    CH_IRQ_EPILOGUE();
}
}

static WORKING_AREA(waMeshLvlThread, 512);
__attribute__ ((__noreturn__))
static void MeshLvlThread(void *arg) {
    chRegSetThreadName("MeshLvl");
    while(1) Mesh.ITask();
}

static WORKING_AREA(waMeshPktBucket, 128);
static void MeshPktBucket(void *arg) {
    chRegSetThreadName("MeshBkt");
    while(1) {
        chEvtWaitAny(EVTMSK_BKT_NOT_EMPTY);
        Mesh.IPktHandler();
    }
}
#endif

void Mesh_t::Init() {
    if(App.ID == 0) {
        Uart.Printf("Msh: WrongID\r");
        return;
    }
    /* Create thread Bucket Handler */
    IPBktHanlder = chThdCreateStatic(waMeshPktBucket, sizeof(waMeshPktBucket), NORMALPRIO, (tfunc_t)MeshPktBucket, NULL);

    UpdateSleepTime();          /* count sleep time due to App.ID */
    MsgBox.Init();
    IGenerateRandomTable(RND_TBL_BUFFER_SZ);

    IResetTimeAge(App.ID, 0);   /* TimeAge = 0; TimeOwner = App.ID */
    IPktPutCycle(AbsCycle);     /* CycleN = AbsCycle */
    Payload.UpdateSelf();  /* Timestamp = AbsCycle; Send info to console */
    PreparePktPayload();

    /* Create Mesh Thread */
    IPThread = chThdCreateStatic(waMeshLvlThread, sizeof(waMeshLvlThread), NORMALPRIO, (tfunc_t)MeshLvlThread, NULL);

    nvicEnableVector(MESH_TIM_IRQ, CORTEX_PRIORITY_MASK(IRQ_PRIO_HIGH));
    CycleTmr.Init(MESH_TIM);
    CycleTmr.SetCounterFreq(1000);
    CycleTmr.SetTopValue(CYCLE_TIME-1);
    CycleTmr.SetCounter(0);
    CycleTmr.EnableIrqOnUpdate();
    IsInit = true;

    chThdSleepMilliseconds(121); /* Do radio thd sleep 41 */

    CycleTmr.Enable();           /* Enable Cycle Timer */
    Uart.Printf("Msh Init ID=%u\r", App.ID);
}

void Mesh_t::ITask() {
    uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);

    if(EvtMsk & EVTMSK_MESH_NEW_CYC) INewCycle();

    if(EvtMsk & EVTMSK_MESH_UPD_CYC) IUpdateTimer();
}

#if 1 // ==== Inner functions ====

void Mesh_t::INewCycle() {
//    Beeper.Beep(BeepShort);
    IIncCurrCycle();
    Payload.UpdateSelf();  /* Timestamp = AbsCycle; Send info to console */
    PreparePktPayload();
//    Uart.Printf("Abs=%u, Curr=%u, RxCyc=%u\r", AbsCycle, CurrCycle, RxCycleN);
//    Uart.Printf("\rNewCyc, t=%u\r", chTimeNow());
    // ==== RX ====
    if(CurrCycle == RxCycleN) {
        chEvtSignal(Radio.rThd, EVTMSK_MESH_RX);
        mshMsg_t MeshPkt;
        do {
            if(MsgBox.TryFetchMsg(&MeshPkt) == OK) {
                IPktHandlerStart();
                PktBucket.WritePkt(MeshPkt);
            }
        } while(Radio.Valets.InRx);
    }
    // ==== TX ====
    else {
//        Uart.Printf("Mesh Tx\r");
        IPktPutCycle(AbsCycle);
        if(SleepTime > 0) chThdSleepMilliseconds(SleepTime);
        chEvtSignal(Radio.rThd, EVTMSK_MESH_TX);
        IWaitTxEnd();
    }
    ITimeAgeCounter();
}

void Mesh_t::IPktHandler(){
    PriorityID = IGetTimeOwner();
    do {
        PktBucket.ReadPkt(&MeshMsg); /* Read Pkt from Buffer */
        MeshPayload_t *pMP = &MeshMsg.MeshPayload;

        if(PriorityID > pMP->TimeOwnerID) {
            IResetTimeAge(PriorityID, 0);
            GetPrimaryPkt = true;
        }

        else if(PriorityID == pMP->TimeOwnerID) {  /* compare TimeAge */
            if(IGetTimeAge() > pMP->TimeAge) {
                IResetTimeAge(PriorityID, pMP->TimeAge);
                GetPrimaryPkt = true;  /* need to update */
            }
        }

        if(pMP->SelfID < STATIONARY_ID) {
            if(MeshMsg.RSSI > PreliminaryRSSI) {
                PreliminaryRSSI = MeshMsg.RSSI;
                Payload.NewLocation(pMP->SelfID);
            }
        };

        if(GetPrimaryPkt) {
            CycleTmr.Disable();
            *PNewCycleN = pMP->CycleN + 1;
            PriorityID = pMP->TimeOwnerID;
            Mesh.PktTx.MeshData.TimeOwnerID = PriorityID;
            *PTimeToWakeUp = MeshMsg.Timestamp + (uint32_t)CYCLE_TIME - (SLOT_TIME * PriorityID);
        }
    } while(PktBucket.GetFilledSlots() != 0);
}

void Mesh_t::IUpdateTimer() {
    PreliminaryRSSI = STATIONARY_MIN_LEVEL;
    if(GetPrimaryPkt) {
        uint32_t timeNow = chTimeNow();
        do {
            *PTimeToWakeUp += CYCLE_TIME;
            *PNewCycleN += 1;
        } while (*PTimeToWakeUp < timeNow);

//        if(*PTimeToWakeUp < timeNow) {
//            *PTimeToWakeUp += CYCLE_TIME;
//            *PNewCycleN += 1;
//        }
        SetCurrCycleN(*PNewCycleN);
        Payload.CorrectionTimeStamp(*PTimeToWakeUp - timeNow);
        CycleTmr.SetCounter(0);
        GetPrimaryPkt = false;
        Uart.Printf("Sleep from %u to %u\r", chTimeNow(), *PTimeToWakeUp);
        if(*PTimeToWakeUp > chTimeNow()) chThdSleepUntil(*PTimeToWakeUp); /* TODO: Thinking carefully about asynch switch on Timer with Virtual timer */
        CycleTmr.Enable();
    }
}

void Mesh_t::PreparePktPayload() {
    PktTx.MeshData.SelfID = App.ID;
    PayloadString_t *PlStr;
    uint16_t NextID = 0;
    NextID = Payload.GetNextInfoID();
    PlStr = Payload.GetInfoByID(NextID);
    PktTx.PayloadID = NextID;
    PktTx.Payload = *PlStr;
//    Uart.Printf("MeshPkt: %u %u %u %d %u %u %u\r",
//            PktTx.PayloadID,
//            PktTx.Payload.Hops,
//            PktTx.Payload.Timestamp,
//            PktTx.Payload.TimeDiff,
//            PktTx.Payload.Reason,
//            PktTx.Payload.Location,
//            PktTx.Payload.Emotion
//            );
}

void Mesh_t::IWaitTxEnd() {
    chSysLock();
    chSchGoSleepS(THD_STATE_SUSPENDED);
    chSysUnlock();
}

void Mesh_t::ITxEnd() {
    chSysLock();
    if(IPThread->p_state == THD_STATE_SUSPENDED) chSchReadyI(IPThread);
    chSysUnlock();
}

void Mesh_t::IIrqHandler() {
    CycleTmr.ClearIrqBits();
    if(IPThread != nullptr) chEvtSignalI(IPThread, EVTMSK_MESH_NEW_CYC);
}
#endif
