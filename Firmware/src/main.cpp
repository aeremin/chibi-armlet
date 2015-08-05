/*
 * File:   main.cpp
 * Author: Kreyl
 * Project: Armlet2South
 *
 * Created on Feb 05, 2013, 20:27
 */

#include "kl_lib.h"
#include "ch.h"
#include "hal.h"
#include "clocking.h"
#include "indication.h"
#include "colors_sounds.h"
#include "pill_mgr.h"
#include "uart.h"
#include "main.h"
#include "radio_lvl1.h"
#include "eestore.h"
#include "evt_mask.h"
#include "adc15x.h"

App_t App;
Eeprom_t EE;

#if 1 // ============================ Timers ===================================
PeriodicTmr_t TmrPillCheck = {MS2ST(T_PILL_CHECK_MS),  EVTMSK_PILL_CHECK};
PeriodicTmr_t TmrDoseSave  = {MS2ST(T_DOSE_SAVE_MS),   EVTMSK_DOSE_SAVE};
PeriodicTmr_t TmrMeasure   = {MS2ST(T_MEASUREMENT_MS), EVTMSK_MEASURE_TIME};
#endif

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V5);
    Clk.UpdateFreqValues();
    // ==== Init OS ====
    halInit();
    chSysInit();

    // ==== Init Hard & Soft ====
    App.InitThread();

    Uart.Init(115200);
    Uart.Printf("\rRst\r\n");
    Indication.Init();
    PillMgr.Init();

    App.Init();
    Radio.Init();

    // Battery measurement
    PinSetupAnalog(GPIOA, 0);
    Adc.InitHardware();
    Adc.PThreadToSignal = chThdSelf();

    // Timers
    TmrPillCheck.InitAndStart(chThdSelf());
    TmrDoseSave.InitAndStart(chThdSelf());
    TmrMeasure.InitAndStart(chThdSelf());

    // ==== Main cycle ====
    App.ITask();
}

// Event-generating framework
__attribute__ ((__noreturn__))
void App_t::ITask() {
    bool PillWasConnected = false;
    while(true) {
        uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
        // ==== Uart cmd ====
        if(EvtMsk & EVTMSK_UART_NEW_CMD) {
            OnUartCmd(&Uart);
            Uart.SignalCmdProcessed();
        }

        // ==== Check pill ====
        if(EvtMsk & EVTMSK_PILL_CHECK) {
            bool IsNowConnected = (PillMgr.CheckIfConnected(PILL_I2C_ADDR) == OK);
            if(IsNowConnected and !PillWasConnected) {  // OnConnect
                PillWasConnected = true;
                OnPillConnect();
            }
            else if(!IsNowConnected and PillWasConnected) PillWasConnected = false;
        } // if EVTMSK_PILL_CHECK
        if(EvtMsk & EVTMSK_PROLONGED_PILL) OnProlongedPill();

        // ==== Dose ====
        if(EvtMsk & EVTMSK_DOSE_SAVE) SaveDose();

        // ==== Measure battery ====
        if(EvtMsk & EVTMSK_MEASURE_TIME) Adc.StartMeasurement();
        if(EvtMsk & EVTMSK_MEASUREMENT_DONE) {
            uint32_t AdcRslt = Adc.GetResult(0);
//            Uart.Printf("Adc=%u\r", AdcRslt);
            Indication.BatteryState = (AdcRslt >= BATTERY_DISCHARGED_ADC)? bsGood : bsBad;
        }

        // ==== EMP ====
        if(EvtMsk & EVTMSK_KEY_POLL)      Grenade.OnKeyPoll();
        if(EvtMsk & EVTMSK_RADIATION_END) Grenade.OnRadiationEnd();

        // ==== Radio ====
        if(EvtMsk & EVTMSK_RX_TABLE_READY) OnRxTableReady();
    } // while true
}



// Timers callbacks prototypes
extern void TmrDoseSaveCallback(void *p) __attribute__((unused));

#if 1 // ======================= Command processing ============================
void App_t::OnUartCmd(Uart_t *PUart) {
    UartCmd_t *PCmd = &PUart->Cmd;
    uint8_t b;
    int32_t dw32 __attribute__((unused));  // May be unused in some cofigurations
    Uart.Printf("\r%S\r", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("Ping")) Uart.Ack(OK);

#if 1 // ==== ID & Type ====
    else if(PCmd->NameIs("SetID")) {
        if(PCmd->GetNextToken() != OK) { PUart->Ack(CMD_ERROR); return; }
        if(PCmd->TryConvertTokenToNumber(&dw32) != OK) { PUart->Ack(CMD_ERROR); return; }
        b = ISetID(dw32);
        PUart->Ack(b);
    }
    else if(PCmd->NameIs("GetID")) Uart.Reply("ID", ID);

    else if(PCmd->NameIs("SetType")) {
        if(PCmd->GetNextToken() != OK) { PUart->Ack(CMD_ERROR); return; }
        if(PCmd->TryConvertTokenToNumber(&dw32) != OK) { PUart->Ack(CMD_ERROR); return; }
        b = ISetType(dw32);
        Uart.Ack(b);
    }
    else if(PCmd->NameIs("GetType")) Uart.Reply("Type", Type);
#endif

#if 1 // ==== Pills ====
    else if(PCmd->NameIs("#PillState")) {
        int32_t PillAddr;
        if(PCmd->GetNextToken() != OK) { PUart->Ack(CMD_ERROR); return; }
        if(PCmd->TryConvertTokenToNumber(&PillAddr) != OK) { PUart->Ack(CMD_ERROR); return; }
        if((PillAddr >= 0) and (PillAddr <= 7)) {
            b = PillMgr.CheckIfConnected(PILL_I2C_ADDR);
            if(b == OK) Uart.Printf("#Pill %u Ok\r\n", PillAddr);
            else Uart.Printf("#Pill %u Fail\r\n", PillAddr);
        }
        else Uart.Ack(CMD_ERROR);
    }

#if 0 // TODO
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

    else if(PCmd->NameIs("#PillRepeatWrite32") and (Type == dtPillFlasher)) {
        uint32_t PillAddr;
        if(PCmd->TryConvertTokenToNumber(&PillAddr) == OK) {
            if((PillAddr >= 0) and (PillAddr <= 7)) {
                b = OK;
                Data2Wr.Sz32 = 0;
                for(uint32_t i=0; i<PILL_SZ32; i++) Data2Wr.Data[i] = 0;
                // Iterate data
                for(uint32_t i=0; i<PILL_SZ32; i++) {
                    if(PCmd->GetNextToken() != OK) break;   // Get next data to write, get out if end
                    //Uart.Printf("%S\r", PCmd->Token);
                    b = PCmd->TryConvertTokenToNumber(&Data2Wr.Data[i]);
                    if(b == OK) Data2Wr.Sz32++;
                    else break; // Token is NAN
                } // while
                // Save data to EEPROM
                if(b == OK) b = EE.WriteBuf(&Data2Wr, sizeof(Data2Wr), EE_REPDATA_ADDR);
                Uart.Ack(b);
                // Write pill immediately if connected
                if(PillMgr.CheckIfConnected(PILL_I2C_ADDR) == OK) App.OnPillConnect();
                return;
            } // if pill addr ok
        } // if pill addr
        Uart.Ack(CMD_ERROR);
    }
#endif
#endif // Pills

#if 0 // ==== Dose ==== TODO
    else if(PCmd->NameIs("#SetDoseTop")) {
        int32_t NewTop;
        if(PCmd->TryConvertTokenToNumber(&NewTop) == OK) {  // Next token is number
            Dose.Consts.Setup(NewTop);
            Dose.SaveTop();
            Uart.Ack(OK);
        }
        else Uart.Ack(CMD_ERROR);
    }
    else if(PCmd->NameIs("#GetDoseTop")) Uart.Printf("#DoseTop %u\r\n", Dose.Consts.Top);

    else if(PCmd->NameIs("#SetDose")) {
        int32_t NewDose;
        if(PCmd->TryConvertTokenToNumber(&NewDose) == OK) {  // Next token is number
            if(NewDose <= Dose.Consts.Top) {
                Dose.Set(NewDose);
                Uart.Ack(OK);
            }
        }
        else Uart.Ack(CMD_ERROR);
    }
    else if(PCmd->NameIs("#GetDose")) Uart.Printf("#Dose %u\r\n", Dose.Value);
#endif // Dose

    else Uart.Ack(CMD_UNKNOWN);
}
#endif

#if 1 // =============================== Radio =================================
void App_t::OnRxTableReady() {
    // Radio damage
    Table_t *PTbl = RxTable.PTable;
    RxTable.dBm2PercentAll();
    if(ANY_OF_3(Type, dtUmvos, dtDetectorMobile, dtDetectorFixed)) {
        int32_t NaturalDmg = 1, RadioDmg = 0;
    //    RxTable.Print();
        // Iterate received levels
        for(uint32_t i=0; i<PTbl->Size; i++) {
            Row_t *PRow = &PTbl->Row[i];
            int32_t rssi = PRow->Rssi;
            if(rssi >= PRow->LvlMin) {    // Only if signal level is enough
                if((PRow->DmgMax == 0x00) and (PRow->DmgMin == 0x00)) NaturalDmg = 0; // "Clean zone"
                else { // ==== Ordinal lustra ====
                    int32_t EmDmg=0, DmgMinReal, DmgMaxReal;
                    // Calculate real damage out of pkt values
                    if(PRow->DmgMax == 0xFF) { // Lethal Lustra
                        DmgMinReal = PRow->DmgMin;
                        DmgMinReal = DmgMinReal * DmgMinReal;   // Damage = Pkt.Damage ^2
                        DmgMaxReal = Dose.Consts.Top;
                    }
                    else {
                        DmgMinReal = PRow->DmgMin;
                        DmgMaxReal = PRow->DmgMax;
                        DmgMinReal = DmgMinReal * DmgMinReal;   // }
                        DmgMaxReal = DmgMaxReal * DmgMaxReal;   // } Damage = Pkt.Damage ^2
                    }
                    // Calculate damage depending on RSSI
                    if(rssi >= PRow->LvlMax) EmDmg = DmgMaxReal;
                    else {
                        int32_t DifDmg = DmgMaxReal - DmgMinReal;
                        int32_t DifLvl = PRow->LvlMax - PRow->LvlMin;
                        EmDmg = (rssi * DifDmg + DmgMaxReal * DifLvl - PRow->LvlMax * DifDmg) / DifLvl;
                        if(EmDmg < 0) EmDmg = 0;
    //                    Uart.Printf("Ch %u; Dmg=%d\r", PRow->Channel, EmDmg);
                    }
                    RadioDmg += EmDmg;
                } // ordinal
            } // if lvl > min
        } // for
        Damage = NaturalDmg + RadioDmg;
        // ==== Damage *= CountOfSecondsElapsedSinceLastTime ====
        uint32_t Age = PTbl->Age();
//        Uart.Printf("Age=%u\r", Age);
        if(Age > 1024) { // More than a second has passed since last time
            uint64_t Dmg64 = Damage;
            uint32_t Dmg32;
            Dmg64 *= Age;
            if(Dmg64 > MAX_UINT32) {
                Dmg64 /= 1024;
                if(Dmg64 > MAX_INT32) Damage = MAX_INT32;
                else Damage = (int32_t)Dmg64;
            }
            else {
                Dmg32 = (uint32_t)Dmg64;
                Dmg32 /= 1000;
                Damage = (int32_t)Dmg32;
            }
        } // if(Age > 1024)
//        Uart.Printf("Dmg=%d\r", Damage);
        // === React depending on device type and damage level ===
        if(Type == dtUmvos) {
            if(Dose.Drug.IsActive()) Dose.Drug.ModifyDamage(Damage, PTbl->Age());
            Dose.Modify(Damage);
        }
    } // if umvos / detector

    else if(Type == dtPelengator) {
        int32_t RssiMax = 0;
        DeviceType_t DevType = dtNothing;
//        Uart.Printf("Sz=%u\r", PTbl->Size);
        for(uint32_t i=0; i<PTbl->Size; i++) {
            Row_t *PRow = &PTbl->Row[i];
            if(PRow->Rssi > RssiMax) {
                RssiMax = PRow->Rssi;
                // Discover device type
                if((PRow->DmgMax == 0x00) and (PRow->DmgMin == 0x00)) DevType = dtLustraClean;
                else if(PRow->DmgMax == 0xFF)  DevType = dtLustraLethal;
                else if(PRow->DmgMax == PktLustra[(dtLustraWeak - dtLustraClean)].DmgMax) DevType = dtLustraWeak;
                else DevType = dtLustraStrong;
            }
        } // for
        Indication.PelengatorDevTypeReceived(DevType);
    } // if pelengator
}
#endif // Dose

#if 1 // ============================= Grenade_t ===============================
void Grenade_t::Init() {
    // Load all
    Charge = EE.Read32(EE_CHARGE_ADDR);
    Capacity = EE.Read32(EE_CAPACITY_ADDR);
    Uart.Printf("Charge=%u; Capacity=%u\r", Charge, Capacity);
    if(Capacity == 0) Capacity = CAPACITY_DEFAULT;
    if(Charge > Capacity) Charge = Capacity;
    // Set State
    if(Charge == Capacity) State = gsReady;
    else State = gsDischarged;
    // Key
    PinSetupIn(KEY_GPIO, KEY_PIN, pudPullUp);
    chVTSet(&TmrKey, MS2ST(T_KEY_POLL_MS), TmrGeneralCallback, (void*)EVTMSK_KEY_POLL);
}
void Grenade_t::DeinitI() {
    if(chVTIsArmedI(&TmrKey)) chVTResetI(&TmrKey);
    if(chVTIsArmedI(&TmrRadiationEnd)) chVTResetI(&TmrRadiationEnd);
}

void Grenade_t::SaveCharge() {
    // To save EEPROM, write only large changes, or top value
    uint32_t OldCharge = EE.Read32(EE_CHARGE_ADDR);
    int32_t Dif = OldCharge - Charge;
    if((Dif>60) or (Dif<-60) or (Charge == Capacity) or (Charge == 0))
        EE.Write32(EE_CHARGE_ADDR, Charge);
}

void Grenade_t::IncreaseCharge() {
    if(Charge < Capacity) Charge++;
    SaveCharge();
}

void Grenade_t::OnKeyPoll() {
    static bool KeyWasPressed = false;
    if(KeyIsPressed()) {
        if(!KeyWasPressed) {
            KeyWasPressed = true;
            if(State == gsReady) {
                Charge = 0;
                SaveCharge();
                State = gsRadiating;
                Indication.JustWakeup();
                // Setup tmr to radiation end
                chVTSet(&TmrRadiationEnd, MS2ST(T_RADIATION_DURATION_MS), TmrGeneralCallback, (void*)EVTMSK_RADIATION_END);
            }
            else if(State != gsRadiating) Beeper.Beep(BeepGrenadeError); // Not ready
        }
    }
    else KeyWasPressed = false;
    // Restart timer to poll Key
    chVTSet(&TmrKey, MS2ST(T_KEY_POLL_MS), TmrGeneralCallback, (void*)EVTMSK_KEY_POLL);
}
#endif

#if 1 // =========================== Emp Mech ==================================
void EmpMech_t::Init() {
    // Output
    PinSetupOut(OUTPUT_GPIO, OUTPUT_PIN, omPushPull, pudNone);
    // Load previous state
    uint32_t tmp = EE.Read32(EE_STATE_ADDR);
    if(tmp > msBroken) tmp = 0;
    SetState((MechState_t)tmp);
}
#endif

#if 1 // ========================= Application =================================
void App_t::Init() {
    ID = EE.Read32(EE_DEVICE_ID_ADDR);  // Read device ID
    ISetType(EE.Read32(EE_DEVICE_TYPE_ADDR));
    Uart.Printf("ID=%u\r\n", ID);
    Damage = 1;
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

uint8_t App_t::ISetType(uint8_t AType) {
    if(AType > dtPillFlasher) return FAILURE;
    if(AType == dtNothing) Type = dtUmvos;
    else Type = (DeviceType_t)AType;
    // Reinit timers
    TmrDoseSave.Stop();
    Grenade.DeinitI();
#if DO_DOSE_SAVE
    if(App.Type == dtUmvos) TmrDoseSave.Start();
#endif

    // Reinit constants
    Dose.LoadTop(); // Read dose constants
    switch(Type) {
        case dtUmvos:
#if DO_DOSE_SAVE
            Dose.LoadValue();
#endif
            break;
        case dtEmpGrenade: Grenade.Init(); break;
        case dtEmpMech: Mech.Init(); break;
        default: break;
    } // switch

    Indication.ProcessTypeChange();
    Uart.Printf("Type=%u\r\n", Type);
    // Save new type in EE
    return EE.Write32(EE_DEVICE_TYPE_ADDR, Type);
}
#endif

