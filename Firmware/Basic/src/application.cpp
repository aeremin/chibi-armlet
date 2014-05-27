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
#include "console.h"

#include <cstdlib>

App_t App;
// Timers callbacks prototypes
extern void TmrDoseSaveCallback(void *p) __attribute__((unused));
extern void TmrMeasurementCallback(void *p) __attribute__((unused));
extern void TmrClickCallback(void *p);

// Table of colors depending on type
#define DEVICETYPE_BLINK_T_MS   999
const LedChunk_t TypeColorTbl[] = {
        {clBlack,   DEVICETYPE_BLINK_T_MS, ckStop}, // dtNothing
        {{0,1,0},   DEVICETYPE_BLINK_T_MS, ckStop}, // dtUmvos
        {clBlue,    DEVICETYPE_BLINK_T_MS, ckStop}, // dtLustraClean
        {clGreen,   DEVICETYPE_BLINK_T_MS, ckStop}, // dtLustraWeak
        {clYellow,  DEVICETYPE_BLINK_T_MS, ckStop}, // dtLustraStrong
        {clRed,     DEVICETYPE_BLINK_T_MS, ckStop}, // dtLustraLethal
        {clMagenta, DEVICETYPE_BLINK_T_MS, ckStop}, // dtDetectorMobile
        {clMagenta, DEVICETYPE_BLINK_T_MS, ckStop}, // dtDetectorFixed
        {clBlack,   DEVICETYPE_BLINK_T_MS, ckStop}, // dtEmpMech
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
    if(Type == dtPillFlasher) {
        uint8_t b = FAILURE;
        // Write pill if data exists
        EE.ReadBuf(&Data2Wr, sizeof(Data2Wr), EE_REPDATA_ADDR);
        if(Data2Wr.Sz32 != 0) {
            Uart.Printf("#PillWrite32 0");
            for(uint8_t i=0; i<Data2Wr.Sz32; i++) Uart.Printf(",%d", Data2Wr.Data[i]);
            Uart.Printf("\r\n");
            b = PillMgr.Write(PILL_I2C_ADDR, PILL_START_ADDR, Data2Wr.Data, sizeof(Data2Wr));
            Uart.Ack(b);
        }
        if(b == OK) {
            Led.StartBlink(LedPillSetupOk);
            Beeper.Beep(BeepPillOk);
        }
        else {
            Led.StartBlink(LedPillBad);
            Beeper.Beep(BeepPillBad);
        }
    }
    else {
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
            // ==== Cure ====
            case PILL_TYPEID_CURE:
                if(Pill.ChargeCnt > 0) {
                    rslt = OK;
                    Pill.ChargeCnt--;
                    rslt = PillMgr.Write(PILL_I2C_ADDR, PILL_START_ADDR, &Pill, sizeof(Pill_t));
                }
                else rslt = FAILURE; // Discharged pill

                if(rslt == OK) {
                    Beeper.Beep(BeepPillOk);
                    Led.StartBlink(LedPillCureOk);
                    // Decrease dose if not dead
                    if(Dose.State != hsDeath) Dose.Decrease(Pill.Value, diNeverIndicate);
                    chThdSleepMilliseconds(2007);    // Let indication to complete
                    Dose.RenewIndication();
                }
                else {
                    Beeper.Beep(BeepPillBad);
                    Led.StartBlink(LedPillBad);
                    chThdSleepMilliseconds(2007);    // Let indication to complete
                }
                break;

            // ==== Set consts ====
            case PILL_TYPEID_SET_DOSETOP:
                Dose.Consts.Setup(Pill.DoseTop);
                SaveDoseTop();
                Uart.Printf("Top=%u; Red=%u; Yellow=%u\r", Dose.Consts.Top, Dose.Consts.Red, Dose.Consts.Yellow);
                Led.StartBlink(LedPillSetupOk);
                Beeper.Beep(BeepPillOk);
                chThdSleepMilliseconds(2007);
                break;

            default:
                Uart.Printf("Unknown Pill\r");
                Beeper.Beep(BeepPillBad);
                break;
        } // switch
    } // if pill flasher
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
            SaveDoseTop();
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
                Dose.Set(NewDose, diAlwaysIndicate);
                Uart.Ack(OK);
            }
        }
        else Uart.Ack(CMD_ERROR);
    }
    else if(PCmd->NameIs("#GetDose")) Uart.Printf("#Dose %u\r\n", Dose.Get());
#endif // Dose

#if 1 // Mesh

    else if(PCmd->NameIs("#SetMeshCycle")) {
        uint32_t NewCycle;
        if(PCmd->TryConvertTokenToNumber(&NewCycle) == OK) {  // Next token is number
//            Uart.Printf("New cycle %u\r", NewCycle);
            Console_SetTime_Ack(Mesh.SetCurrCycleN(NewCycle));
        }
    }

    else if(PCmd->NameIs("#GetMeshInfo")) {
        Console_GetMeshInfo_Ack(OK);
    }
#endif // Mesh

    else if(*PCmd->Name == '#') Uart.Ack(CMD_UNKNOWN);  // reply only #-started stuff
}
#endif

#if 1 // =============================== Dose ==================================
void App_t::OnRxTableReady() {
//    uint32_t TimeElapsed = chTimeNow() - LastTime;
//    LastTime = chTimeNow();
    // Radio damage
    Table_t *PTbl = RxTable.PTable;
    uint32_t NaturalDmg = 1, RadioDmg = 0;
    RxTable.dBm2PercentAll();
//    RxTable.Print();
    // Iterate received levels
    for(uint32_t i=0; i<PTbl->Size; i++) {
        Row_t *PRow = &PTbl->Row[i];
        int32_t rssi = PRow->Rssi;
        if(rssi >= PRow->LvlMin) {    // Only if signal level is enough
            if((PRow->DmgMax == 0) and (PRow->DmgMin == 0)) NaturalDmg = 0; // "Clean zone"
            else { // Ordinal lustra
                int32_t EmDmg = 0;
                if(rssi >= PRow->LvlMax) EmDmg = PRow->DmgMax;
                else {
                    int32_t DifDmg = PRow->DmgMax - PRow->DmgMin;
                    int32_t DifLvl = PRow->LvlMax - PRow->LvlMin;
                    EmDmg = (rssi * DifDmg + PRow->DmgMax * DifLvl - PRow->LvlMax * DifDmg) / DifLvl;
                    if(EmDmg < 0) EmDmg = 0;
//                    Uart.Printf("Ch %u; Dmg=%d\r", PRow->Channel, EmDmg);
                }
                RadioDmg += EmDmg;
            } // ordinal
        } // if lvl > min
    } // for
    Damage = NaturalDmg + RadioDmg;
//    Uart.Printf("Total=%d\r", Damage);
    if(Type == dtUmvos) {
        Dose.Increase(Damage, diUsual);
        Uart.Printf("Dz=%u; Dmg=%u\r", Dose.Get(), Damage);
    }
}

void App_t::OnClick() {
    if(Damage > 0) {
        int32_t r = rand() % (DMG_SND_MAX - 1);
        int32_t DmgSnd = (((DMG_SND_MAX - DMG_SND_BCKGND) * (Damage - 1)) / (DMG_MAX - 1)) + DMG_SND_BCKGND;
//        Uart.Printf("%d; %d\r", Damage, DmgSnd);
        if(r < DmgSnd) {
            TIM2->CR1 = TIM_CR1_CEN | TIM_CR1_OPM;
            Led.StartBlink(LedClick);
            return;
        }
    }
    Led.SetColor(CLR_NO_DAMAGE);
}
#endif // Dose

#if 1 // ========================= Application =================================
void App_t::Init() {
    ID = EE.Read32(EE_DEVICE_ID_ADDR);  // Read device ID
    ISetType(EE.Read32(EE_DEVICE_TYPE_ADDR));
    Uart.Printf("ID=%u\r\n", ID);
    Damage = 1; // DEBUG
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
    if(chVTIsArmedI(&TmrDoseSave))    chVTResetI(&TmrDoseSave);
    if(chVTIsArmedI(&TmrMeasurement)) chVTResetI(&TmrMeasurement);
    if(chVTIsArmedI(&TmrClick))       chVTResetI(&TmrClick);
    switch(App.Type) {
        case dtUmvos:
//            chVTSetI(&TmrMeasurement, MS2ST(TM_MEASUREMENT_MS),   TmrMeasurementCallback, nullptr);
#if DO_DOSE_SAVE
            chVTSetI(&TmrDoseSave,    MS2ST(TM_DOSE_SAVE_MS),     TmrDoseSaveCallback, nullptr);
#endif
            break;
        case dtDetectorMobile:
            chVTSetI(&TmrClick, MS2ST(TM_CLICK), TmrClickCallback, nullptr);
            break;
        default: break;
    }
    chSysUnlock();

    // Reinit constants
    uint32_t tmp1;
    switch(Type) {
        case dtUmvos:
            // Read dose constants
            tmp1 = EE.Read32(EE_DOSETOP_ADDR);
            if(tmp1 == 0) tmp1 = DOSE_DEFAULT_TOP;  // In case of clear EEPROM, use default value
            Dose.Consts.Setup(tmp1);
            Uart.Printf("Top=%u; Red=%u; Yellow=%u\r\n", Dose.Consts.Top, Dose.Consts.Red, Dose.Consts.Yellow);
#if DO_DOSE_SAVE
            Dose.Load();
#endif
            break;

        case dtDetectorMobile:
            rccEnableTIM2(FALSE);
            PinSetupAlterFunc(GPIOB, 3, omPushPull, pudNone, AF1);
            TIM2->CR1 = TIM_CR1_CEN | TIM_CR1_OPM;
            TIM2->CR2 = 0;
            TIM2->ARR = 22;
            TIM2->CCMR1 |= (0b111 << 12);
            TIM2->CCER  |= TIM_CCER_CC2E;
            tmp1 = TIM2->ARR * 1000;
            tmp1 = Clk.APB1FreqHz / tmp1;
            if(tmp1 != 0) tmp1--;
            TIM2->PSC = (uint16_t)tmp1;
            TIM2->CCR2 = 20;
            break;

        default: break;
    } // switch

    Led.StartBlink(&TypeColorTbl[AType]);   // Blink with correct color
    // Save in EE if not equal
    uint32_t EEType = EE.Read32(EE_DEVICE_TYPE_ADDR);
    uint8_t rslt = OK;
    if(EEType != Type) rslt = EE.Write32(EE_DEVICE_TYPE_ADDR, Type);
    Uart.Printf("Type=%u\r\n", Type);
    return rslt;
}
#endif
