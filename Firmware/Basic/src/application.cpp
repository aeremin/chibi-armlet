/*
 * application.cpp
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#include "application.h"
#include "cmd_uart.h"
#include "pill_mgr.h"
#include "peripheral.h"
#include "evt_mask.h"
#include "eestore.h"
#include "radio_lvl1.h"
#include "adc15x.h"

#include "mesh_lvl.h"
#include "RxTable.h"
#include "console.h"
#include "reasons.h"
#include <cstdlib>

#include "led_rgb.h"
App_t App;
// Timers callbacks prototypes
extern void TmrMeasurementCallback(void *p) __attribute__((unused));

#if 1 // ================================ Pill =================================
void App_t::OnPillConnect() {
    if(PillMgr.Read(PILL_I2C_ADDR, PILL_START_ADDR, &Pill, sizeof(Pill_t)) != OK) return;
    // Print pill
    Uart.Printf("#PillRead32 0 16\r\n");
    Uart.Printf("#PillData32 ");
    int32_t *p = (int32_t*)&Pill;
    for(uint32_t i=0; i<PILL_SZ32; i++) Uart.Printf("%d ", *p++);
    Uart.Printf("\r\n");
    uint8_t rslt __attribute__((unused));
    switch(Pill.TypeID) {
#if 1 // ==== Set ID ====
        case PILL_TYPEID_SET_ID:
            Pill.DeviceID++;
            rslt = PillMgr.Write(PILL_I2C_ADDR, 0, &Pill, PILL_SZ);
            if(rslt == OK) {
                ISetID(Pill.DeviceID-1);
                Led.StartSequence(LedGoodID);
                Beeper.Beep(BeepPillOk);
            }
            else {
                Uart.Printf("Pill Write Error\r");
                Led.StartSequence(LedBadID);
                Beeper.Beep(BeepPillBad);
            }
            chThdSleepMilliseconds(1800);
            break;
#endif

        default:
            Uart.Printf("Unknown Pill\r");
            Beeper.Beep(BeepPillBad);
            break;
    } // switch
}
#endif

#if 1 // ======================= Command processing ============================
void App_t::OnUartCmd(Cmd_t *PCmd) {
//    Uart.Printf("%S\r", PCmd->Name);
    uint8_t b;
    uint32_t dw32 __attribute__((unused));  // May be unused in some cofigurations
    if(PCmd->NameIs("#Ping")) Uart.Ack(OK);

#if 1 // ==== ID & Type ====
    else if(PCmd->NameIs("#SetID")) {
        if(PCmd->TryConvertTokenToNumber(&dw32) == OK) {  // Next token is number
            b = ISetID(dw32);
            Mesh.UpdateSleepTime();
            Uart.Ack(b);
        }
        else Uart.Ack(CMD_ERROR);
    }
    else if(PCmd->NameIs("#GetID")) Uart.Printf("#ID %u\r\n", SelfID);
#endif

#if 1 // ==== Pills ====
    else if(PCmd->NameIs("#PillState")) {
        uint32_t PillAddr;
        if(PCmd->TryConvertTokenToNumber(&PillAddr) == OK) {
            if((PillAddr >= 0) and (PillAddr <= 7)) {
                b = PillMgr.CheckIfConnected(PILL_I2C_ADDR);
                if(b == OK) Uart.Printf("#Pill %u Ok\r\n", PillAddr);
                else Uart.Printf("#Pill %u Fail\r\n", PillAddr);
                return;
            }
        }
        Uart.Ack(CMD_ERROR);
    }

    else if(PCmd->NameIs("#PillRead32")) {
        uint32_t PillAddr;
        if(PCmd->TryConvertTokenToNumber(&PillAddr) == OK) {
            if((PillAddr >= 0) and (PillAddr <= 7)) {
                PCmd->GetNextToken();
                uint32_t Cnt = 0;
                uint8_t MemAddr = PILL_START_ADDR;
                if(PCmd->TryConvertTokenToNumber(&Cnt) == OK) {
                    Uart.Printf("#PillData32 ");
                    for(uint32_t i=0; i<Cnt; i++) {
                        b = PillMgr.Read(PILL_I2C_ADDR, MemAddr, &dw32, 4);
                        if(b != OK) break;
                        Uart.Printf("%d ", dw32);
                        MemAddr += 4;
                    }
                    Uart.Printf("\r\n");
                    if(b != OK) Uart.Ack(b);
                    return;
                } // if data cnt
            } // if pill addr ok
        } // if pill addr
        Uart.Ack(CMD_ERROR);
    }

    else if(PCmd->NameIs("#PillWrite32")) {
        uint32_t PillAddr;
        if(PCmd->TryConvertTokenToNumber(&PillAddr) == OK) {
            if((PillAddr >= 0) and (PillAddr <= 7)) {
                b = OK;
                uint8_t MemAddr = PILL_START_ADDR;
                // Iterate data
                while(true) {
                    PCmd->GetNextToken();   // Get next data to write
                    if(PCmd->TryConvertTokenToNumber(&dw32) != OK) break;
//                    Uart.Printf("%X ", Data);
                    b = PillMgr.Write(PILL_I2C_ADDR, MemAddr, &dw32, 4);
                    if(b != OK) break;
                    MemAddr += 4;
                } // while
                Uart.Ack(b);
                return;
            } // if pill addr ok
        } // if pill addr
        Uart.Ack(CMD_ERROR);
    }
#endif // Pills

#if 1 // Mesh
    else if(PCmd->NameIs("#SetMeshCycle")) {
        uint32_t NewCycle;
        if(PCmd->TryConvertTokenToNumber(&NewCycle) == OK) {  // Next token is number
//            Uart.Printf("New cycle %u\r", NewCycle);
            Console.SetTime_Ack(Mesh.SetNewAbsCycleN(NewCycle));
        }
    }
    else if(PCmd->NameIs("#GetMeshInfo")) {
        Console.GetMeshInfo_Ack(OK);
    }
#endif // Mesh

    else if(*PCmd->Name == '#') Uart.Ack(CMD_UNKNOWN);  // reply only #-started stuff
}
#endif

#if 1 // =============================== Mesh ==================================
void App_t::OnRxTableReady() {
//	RxTable.PTable->Print();    // Debug
    uint8_t LocSignalPwr = 0;
    uint8_t ReasonSigPwr = 0;
    uint16_t LocationID = 0;
    uint16_t NeighborID = 0;
    uint16_t tmpID=0;
    for(uint32_t i=0; i<RxTable.PTable->Size; i++) {
        tmpID = RxTable.PTable->Row[i].ID;
        if((tmpID >= LOCATION_ID_START) && (LOCATIONS_ID_END >= tmpID))    {
            if(RxTable.PTable->Row[i].Level > LocSignalPwr) {
                LocSignalPwr = RxTable.PTable->Row[i].Level;
                LocationID = tmpID;
            } // if Signal present
        } // if location correct
        else if((tmpID >= PERSON_ID_START) && (PERSON_ID_END >= tmpID)) {
            if(RxTable.PTable->Row[i].Level > ReasonSigPwr) {
                ReasonSigPwr = RxTable.PTable->Row[i].Level;
                NeighborID = tmpID;
            } // if Signal present
        }
    }
    if(LocationID) {
        CurrInfo.Location = LocationID;
        LocationValid();
    }
    else LocationInvalid();
    if(NeighborID) {
        CurrInfo.Neighbor = NeighborID;
    }
    else CurrInfo.Neighbor = 0;
}
#endif

#define RLEVEL_MIN_TO_GREENLIGHT    72  // 63: 15-25m; 81: 4m;
#define LOSS_TIMEOUT                9999
void App_t::CheckIfLightGreen() {
    // Iterate RxTable
    static uint32_t TimestampSomeone = 0;
    bool SomeoneIsNear = false;
    for(uint32_t i=0; i < RxTable.PTable->Size; i++) {
        if(RxTable.PTable->Row[i].Level < RLEVEL_MIN_TO_GREENLIGHT) continue;   // Ignore weak signals
//        uint32_t ID = RxTable.PTable->Row[i].ID;
                SomeoneIsNear = true;
                break;
    } // for
    if(SomeoneIsNear) {
        TimestampSomeone = chTimeNow();
        if(Led.GetCurrentSequence() != LedSomeoneIsNear) Led.StartSequence(LedSomeoneIsNear);
    }
    else {
        // Check if enough time passed since last someone
        if((chTimeNow() - TimestampSomeone) > LOSS_TIMEOUT) {
            if(Led.GetCurrentSequence() != LedNobodyHere) Led.StartSequence(LedNobodyHere);
        }
    }
}

#if 1 // ========================= Application =================================
void App_t::Init() {
    SelfID = EE.Read32(EE_DEVICE_ID_ADDR);  // Read device ID
}

uint8_t App_t::ISetID(uint32_t NewID) {
    if(NewID > 0xFFFF) return FAILURE;
    uint8_t rslt = EE.Write32(EE_DEVICE_ID_ADDR, NewID);
    if(rslt == OK) {
        SelfID = NewID;
        Uart.Printf("\r\nNew ID: %u", SelfID);
        return OK;
    }
    else {
        Uart.Printf("\r\nEE error: %u", rslt);
        return FAILURE;
    }
}
#endif
