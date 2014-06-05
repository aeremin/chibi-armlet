/*
 * application.cpp
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#include "application.h"
#include "colors_sounds.h"
#include "cmd_uart.h"
#include "pill_mgr.h"
#include "indication.h"
#include "evt_mask.h"
#include "eestore.h"
#include "radio_lvl1.h"
#include "adc15x.h"
#include <cstdlib>

App_t App;
Eeprom_t EE;

// Timers callbacks prototypes
extern void TmrDoseSaveCallback(void *p) __attribute__((unused));

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
            Uart.Ack(b);
        }
        else Uart.Ack(CMD_ERROR);
    }
    else if(PCmd->NameIs("#GetID")) Uart.Printf("#ID %u\r\n", ID);

    else if(PCmd->NameIs("#SetType")) {
        if(PCmd->TryConvertTokenToNumber(&dw32) == OK) {  // Next token is number
            b = ISetType(dw32);
            Uart.Ack(b);
        }
        else Uart.Ack(CMD_ERROR);
    }
    else if(PCmd->NameIs("#GetType")) Uart.Printf("#Type %u\r\n", Type);
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
#endif // Pills

#if 1 // ==== Dose ====
    else if(PCmd->NameIs("#SetDoseTop")) {
        int32_t NewTop;
        if(PCmd->TryConvertTokenToNumber(&NewTop) == OK) {  // Next token is number
            Dose.Consts.Setup(NewTop);
            Dose.SaveTop();
            Uart.Printf("Top=%d; Red=%d; Yellow=%d\r", Dose.Consts.Top, Dose.Consts.Red, Dose.Consts.Yellow);
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

    else if(PCmd->Name[0] == '#') Uart.Ack(CMD_UNKNOWN);  // reply only #-started stuff
}
#endif

#if 1 // =============================== Radio =================================
void App_t::OnRxTableReady() {
    // Radio damage
    Table_t *PTbl = RxTable.PTable;
    int32_t NaturalDmg = 1, RadioDmg = 0;
    RxTable.dBm2PercentAll();
//    Uart.Printf("Age=%u\r", PTbl->Age());
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
                if((PRow->DmgMax == 0xFF) and (PRow->DmgMin == 0xFF)) { // Lethal Lustra
                    DmgMinReal = 1;
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
    // TODO: Damage *= CountOfSecondsElapsedSinceLastTime
//    Uart.Printf("Dmg=%d\r", Damage);
    // React depending on device type and damage level
    if(Type == dtUmvos) {
        if(Dose.Drug.IsActive()) Dose.Drug.ModifyDamage(Damage, PTbl->Age());
        Dose.Modify(Damage);
    }
}
#endif // Dose

#if 1 // ============================= Grenade_t ===============================
void Grenade_t::Init() {
    // Load all
    Charge = EE.Read32(EE_CHARGE_ADDR);
    Capacity = EE.Read32(EE_CAPACITY_ADDR);
    Uart.Printf("Charge=%u; Capacity=%u\r", Charge, Capacity);
    if(Capacity == 0) { // Empty EEPROM
        Charge = 0;
        Capacity = CAPACITY_DEFAULT;
    }
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
    if(EE.Read32(EE_CHARGE_ADDR) != Charge)
        // To save EEPROM, write only every 10th charge, or top value
        if(((Charge % 10) == 0) or (Charge == Capacity))
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
    // Load previous state
    uint32_t tmp = EE.Read32(EE_STATE_ADDR);
    if(tmp > msBroken) tmp = 0;
    State = (MechState_t)tmp;
    //Todo: Output
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
    Type = (DeviceType_t)AType;
    // Reinit timers
    chSysLock();
    if(chVTIsArmedI(&TmrDoseSave)) chVTResetI(&TmrDoseSave);
    Grenade.DeinitI();
    switch(App.Type) {
        case dtUmvos:
#if DO_DOSE_SAVE
            chVTSetI(&TmrDoseSave, MS2ST(TM_DOSE_SAVE_MS), TmrDoseSaveCallback, nullptr);
#endif
            break;
        default: break;
    }
    chSysUnlock();

    // Reinit constants
    switch(Type) {
        case dtUmvos:
            Dose.LoadTop(); // Read dose constants
#if DO_DOSE_SAVE
            Dose.LoadValue();
#endif
            break;
        case dtEmpGrenade: Grenade.Init(); break;
        case dtEmpMech: Mech.Init(); break;
        default: break;
    } // switch

    Indication.ProcessTypeChange();

    // Save in EE if not equal
    uint32_t EEType = EE.Read32(EE_DEVICE_TYPE_ADDR);
    uint8_t rslt = OK;
    if(EEType != Type) rslt = EE.Write32(EE_DEVICE_TYPE_ADDR, Type);
    Uart.Printf("Type=%u\r\n", Type);
    return rslt;
}
#endif

