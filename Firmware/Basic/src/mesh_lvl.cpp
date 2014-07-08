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

static WORKING_AREA(waMeshPktHandlerThd, 128);
static void MeshPktHandlerThd(void *arg) {
    chRegSetThreadName("MeshPktThd");
    while(1) {
        mshMsg_t MeshPkt;
        chEvtWaitAny(EVTMSK_MESH_PKT_RDY);
        do {
            if(Mesh.MsgBox.TryFetchMsg(&MeshPkt) == OK) {
                Mesh.PktBucket.WritePkt(MeshPkt);
                Mesh.SendEvent(EVTMSK_MESH_PKR_HDL);
            }
        } while(Radio.Valets.InRx);
    }
}
#endif

void Mesh_t::Init() {
    if(App.ID == 0) {
        Uart.Printf("Msh: WrongID\r");
        return;
    }
    /* Create thread Bucket Handler */
    IPPktHanlderThread = chThdCreateStatic(waMeshPktHandlerThd, sizeof(waMeshPktHandlerThd), NORMALPRIO, (tfunc_t)MeshPktHandlerThd, NULL);

    UpdateSleepTime();          /* count sleep time due to App.ID */
    MsgBox.Init();
    IGenerateRandomTable(RND_TBL_BUFFER_SZ);

    IResetTimeAge(App.ID, 0);   /* TimeAge = 0; TimeOwner = App.ID */
    IPktPutCycle(AbsCycle);     /* CycleN = AbsCycle */
    AlienTable.UpdateSelf(AbsCycle);  /* Timestamp = AbsCycle; Send info to console */
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

    chThdSleepMilliseconds(121); /* Do radio thd sleep 41 */  //TODO: define sleep time

    CycleTmr.Enable();           /* Enable Cycle Timer */
    Uart.Printf("[mesh_lvl.cpp] initID=%u\r\n", App.ID);
}

void Mesh_t::ITask() {
    uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);

    if(EvtMsk & EVTMSK_MESH_NEW_CYC) INewCycle();

    else if(EvtMsk & EVTMSK_MESH_RX_END) IUpdateTimer();

    else if(EvtMsk & EVTMSK_MESH_PKR_HDL) IPktHandler();
}

#if 1 // ==== Inner functions ====

void Mesh_t::INewCycle() {
//    Beeper.Beep(BeepShort);
    IIncCurrCycle();
    ITimeAgeCounter();
    AlienTable.UpdateSelf(AbsCycle);  /* Timestamp = AbsCycle; Send info to console */
//    Uart.Printf("Abs=%u, Curr=%u, RxCyc=%u\r", AbsCycle, CurrCycle, RxCycleN);
//    Uart.Printf("\rNewCyc, t=%u\r", chTimeNow());
    // ==== RX ====
    if(CurrCycle == RxCycleN) chEvtSignal(Radio.rThd, EVTMSK_MESH_RX);
    // ==== TX ====
    else {
//        Uart.Printf("Mesh Tx\r");
        PreparePktPayload();
        if(SleepTime > 0) chThdSleepMilliseconds(SleepTime);
        chEvtSignal(Radio.rThd, EVTMSK_MESH_TX);
        IWaitTxEnd();
    }
}

void Mesh_t::IPktHandler(){
    // TODO: what shall i need to do if RxEnd and Time was updated yet?
    PriorityID = IGetTimeOwner();
    if(PktBucket.GetFilledSlots() == 0) return;
    while(PktBucket.GetFilledSlots() != 0) {
        PktBucket.ReadPkt(&MeshMsg); /* Read Pkt from Buffer */
//        Uart.Printf("Extruct Msg -> %u %u %u %u %u %u %u  {%u %u %u %d %u %u %u} \r",
//                MeshMsg.RadioPkt.MeshData.SelfID,
//                MeshMsg.RadioPkt.MeshData.CycleN,
//                MeshMsg.RadioPkt.MeshData.TimeOwnerID,
//                MeshMsg.RadioPkt.MeshData.TimeAge,
//                MeshMsg.RadioPkt.MeshData.SelfReason,
//                MeshMsg.RadioPkt.MeshData.SelfLocation,
//                MeshMsg.RadioPkt.MeshData.SelfEmotion,
//                MeshMsg.RadioPkt.PayloadID,
//                MeshMsg.RadioPkt.Payload.Hops,
//                MeshMsg.RadioPkt.Payload.Timestamp,
//                MeshMsg.RadioPkt.Payload.TimeDiff,
//                MeshMsg.RadioPkt.Payload.Reason,
//                MeshMsg.RadioPkt.Payload.Location,
//                MeshMsg.RadioPkt.Payload.Emotion
//        );

        /* Put information from Pkt */
        AlienTable.PutSender(AbsCycle, &MeshMsg.RadioPkt.SenderInfo);
        uint32_t CycleDiff = (AbsCycle < MeshMsg.RadioPkt.SenderInfo.Mesh.CycleN)? MeshMsg.RadioPkt.SenderInfo.Mesh.CycleN - AbsCycle : AbsCycle - MeshMsg.RadioPkt.SenderInfo.Mesh.CycleN;
        AlienTable.PutAlien(MeshMsg.RadioPkt.AlienID, CycleDiff, &MeshMsg.RadioPkt.AlienIfo);

        /* Dispatch Pkt */

       sender_mesh_t *pSM = &MeshMsg.RadioPkt.SenderInfo.Mesh;
       if( (pSM->TimeOwnerID < PriorityID) ||
               ((pSM->TimeOwnerID == PriorityID) &&
                   (pSM->TimeAge < IGetTimeAge())) ) {  /* compare TimeAge */
           CycleTmr.Disable();
           GetPrimaryPkt = true;                        // received privilege pkt
           PriorityID = pSM->TimeOwnerID;
           IResetTimeAge(PriorityID, pSM->TimeAge);
           *PNewCycleN = pSM->CycleN;   // TODO: cycle number increment: nedeed of not?
           *PTimeToWakeUp = MeshMsg.Timestamp - MESH_PKT_TIME - (SLOT_TIME*(PriorityID-1)) + CYCLE_TIME;
//           *PTimeToWakeUp = (CYCLE_TIME - (SLOT_TIME * PriorityID)) + MeshMsg.Timestamp;
       }

       if(pSM->SelfID <= STATIONARY_ID) { /* if pkt received from lustra */
           if(MeshMsg.RSSI > PreliminaryRSSI) {
               PreliminaryRSSI = MeshMsg.RSSI;
               /* Send Event to App Like an New Raio Location */
//               Payload.NewLocation(pSM->SelfID);
//               PktTx.MeshData.SelfLocation = (pSM->SelfID);
           }
       };
    }
}

void Mesh_t::IUpdateTimer() {
    PreliminaryRSSI = STATIONARY_MIN_LEVEL;
    if(GetPrimaryPkt) {
        uint32_t timeNow = chTimeNow();
//        Uart.Printf("timeNow=%u, WupTime=%u\r", timeNow, *PTimeToWakeUp);
        while(*PTimeToWakeUp < timeNow) {
            *PTimeToWakeUp += CYCLE_TIME;
            *PNewCycleN += 1;
        }

        SetNewAbsCycleN(*PNewCycleN);
        AlienTable.TimeCorrection(*PTimeToWakeUp - timeNow);
        CycleTmr.SetCounter(0);
        GetPrimaryPkt = false;
//        Uart.Printf("Sleep from %u to %u\r", chTimeNow(), *PTimeToWakeUp);
        if(*PTimeToWakeUp > chTimeNow()) chThdSleepUntil(*PTimeToWakeUp); /* TODO: Thinking carefully about asynch switch on Timer with Virtual timer */
        else Uart.Printf("[mesh_lvl.cpp] WT!\r\n");
        CycleTmr.Enable();
    }
}

void Mesh_t::PreparePktPayload() {
    PktTx.SenderInfo.Mesh.SelfID = App.ID;         /* TODO: need if App.ID changed by the Uart command */
    IPktPutCycle(AbsCycle);
    AlienInfo_t *AlienDataStr;
    uint16_t NextID = 0;
    NextID = AlienTable.GetID();
    AlienDataStr = AlienTable.GetInfo(NextID);
    PktTx.AlienID = NextID;
    PktTx.AlienIfo = *AlienDataStr;
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
