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
#include "emotions.h"
#include <cstdlib>

App_t App;
// Timers callbacks prototypes
extern void TmrMeasurementCallback(void *p) __attribute__((unused));

// Table of colors depending on type
#define DEVICETYPE_BLINK_T_MS   999
const LedChunk_t TypeColorTbl[] = {
        {clCyan,    DEVICETYPE_BLINK_T_MS, ckStop}, // dtEmpGrenade
        {clWhite,   DEVICETYPE_BLINK_T_MS, ckStop}, // dtPelengator
        {clBlack,   DEVICETYPE_BLINK_T_MS, ckStop}, // dtPillFlasher
};

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
#if 0 // ==== Set ID ====
            case PILL_TYPEID_SET_ID:
                if(ID == 0) {
                    Pill.DeviceID++;
                    rslt = PillMgr.Write(PILL_I2C_ADDR, &Pill, (sizeof(Pill.TypeID) + sizeof(Pill.DeviceID)));
                    if(rslt == OK) {
                        ISetID(Pill.DeviceID-1);
                        Led.StartBlink(LedPillIdSet);
                        Beeper.Beep(BeepPillOk);
                    }
                    else {
                        Uart.Printf("Pill Write Error\r");
                        Led.StartBlink(LedPillIdNotSet);
                        Beeper.Beep(BeepPillBad);
                    }
                }
                else {
                    Uart.Printf("ID already set: %u\r", ID);
                    Led.StartBlink(LedPillIdNotSet);
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
    else if(PCmd->NameIs("#GetID")) Uart.Printf("#ID %u\r\n", ID);
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
    Uart.Printf("\rOnRxTable");
	RxTable.PTable->Print();    // Debug
	CheckIfLightGreen();
    //mist logic
    //если я не туман, и если я локация

//#define MIST_ID_START 90
//#define MIST_ID_END 100
#ifdef MIST_SUPPORT_CHIBI

    send_info.Location=(uint16_t)0;
	uint8_t SignalPwr = 0;
	    uint8_t LocationID = 0;
	    uint16_t tmpID=0;
	    for(uint32_t i=0; i<RxTable.PTable->Size; i++) {
	        tmpID = RxTable.PTable->Row[i].ID;
	        if( (tmpID >= LOCATION_ID_START && tmpID <= FOREST_ID_END) ||
	            (tmpID >= EMOTION_FIX_ID_START && tmpID <= EMOTION_FIX_ID_END) )    {
	            if(RxTable.PTable->Row[i].Level > SignalPwr) {
	                SignalPwr = RxTable.PTable->Row[i].Level;
	                LocationID = tmpID;
	            } // if Signal present
	        } // if location correct
	    }
	    if(LocationID) send_info.Location = LocationID;

    send_info.Reason=(uint16_t)0;
    send_info.Emotion = 0;  // Lustra has no emotion


    bool is_tuman_incoming=false;
    for(uint32_t i=0;i<RxTable.PTable->Size;i++)
    {
        if(RxTable.PTable->Row[i].State.Reason==(uint16_t)REASON_MSOURCE)
            is_tuman_incoming=true;
    }
    //логика люстр, слушающих туман
    if(App.ID>=LOCATION_IN_GAME_ID_START)
        if(!(App.ID>=MIST_ID_START && App.ID<=MIST_ID_END))
{
    //mesh l
    int32_t timeaddmillisec=S_CYCLE_TIME;
    //если туман активен - тикать!
    if(App.mist_msec_ctr>0)
        App.mist_msec_ctr+=timeaddmillisec;
    //если давно не приходил
    if(App.mist_msec_ctr>(int32_t)MIST_TRANSLATE_TIME_SEC*(int32_t)1000)
    {
        App.mist_msec_ctr=-1;
        //вернуть резон
        send_info.Reason=App.reason_saved;
    }
    //если пришел туман - включить. если уже не было включено - сохранить старый резон

    if(is_tuman_incoming)
    {
        if( App.mist_msec_ctr==-1)//save old reason
        {
              App.reason_saved=send_info.Reason;
        }
        App.mist_msec_ctr=0;
        send_info.Reason=(uint16_t)REASON_MPROJECT;
    }
}
        //если я игротехник страха с mscource, я его излучаю
        if(App.ID>=MIST_ID_START && App.ID<=MIST_ID_END)
        {
            send_info.Reason=(uint16_t)REASON_MSOURCE;

        }
#endif
}

void App_t::CheckIfLightGreen() {
    for(uint32_t i=0; i < RxTable.PTable->Size; i++) {
        uint32_t ID = RxTable.PTable->Row[i].ID;
        if(ID >= CHARACTER_ID_START and ID <= CHARACTER_ID_END) {
            // Light Up
//            Led.IPFirstChunk
            chSysLock();
//            chVTSetI(&ITmr, MS2ST(PSequence->Time_ms), BeeperTmrCallback, (void*)PCh);
            chSysUnlock();
        }
    } // for
}
#endif // Mesh

#if 1 // ========================= Application =================================
void App_t::Init() {
    ID = EE.Read32(EE_DEVICE_ID_ADDR);  // Read device ID
#ifdef MIST_SUPPORT_CHIBI
    mist_msec_ctr=-1;
#endif
}

uint8_t App_t::ISetID(uint32_t NewID) {
    if(NewID > 0xFFFF) return FAILURE;
    uint8_t rslt = EE.Write32(EE_DEVICE_ID_ADDR, NewID);
    if(rslt == OK) {
        ID = NewID;
        Uart.Printf("New ID: %u\r", ID);
        return OK;
    }
    else {
        Uart.Printf("EE error: %u\r", rslt);
        return FAILURE;
    }
}
#endif
