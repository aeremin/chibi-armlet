/*
 * pill.cpp
 *
 *  Created on: 22 мая 2014 г.
 *      Author: g.kruglov
 */

#include "application.h"
#include "pill_mgr.h"
#include "indication.h"

#if 1 // ================================ On Connect ===========================
uint8_t App_t::IPillHandlerUmvos() {
    uint8_t rslt = OK;
    switch(Pill.Type) {
        // ==== Cure ====
        case ptCure:
            if(Pill.ChargeCnt > 0) {    // Check charge count, decrease it and write it back
                Pill.ChargeCnt--;
                rslt = PillMgr.Write(PILL_I2C_ADDR, (PILL_START_ADDR + PILL_CHARGECNT_ADDR), &Pill.ChargeCnt, sizeof(Pill.ChargeCnt));
                // Modify dose if not dead
                if((rslt == OK) and (Dose.State != hsDeath)) Dose.Modify(Pill.Value);
            }
            else rslt = FAILURE;
            break;

        // ==== Drug ====
        case ptDrug:
            if(Pill.ChargeCnt > 0) {
                Pill.ChargeCnt--;
                rslt = PillMgr.Write(PILL_I2C_ADDR, (PILL_START_ADDR + PILL_CHARGECNT_ADDR), &Pill.ChargeCnt, sizeof(Pill.ChargeCnt));
                // Apply drug if not dead
                if((rslt == OK) and (Dose.State != hsDeath)) Dose.Drug.Set(Pill.Value, Pill.Time_s);
            }
            else rslt = FAILURE;
            break;

        // ==== Panacea ====
        case ptPanacea: Dose.Reset(); break;

        // ==== Autodoc ====
        case ptAutodoc: rslt = OnProlongedPill(); break;

        // ==== Set DoseTop ====
        case ptSetDoseTop:
            Dose.Consts.Setup(Pill.DoseTop);
            Dose.SaveTop();
            break;

        // ==== Diagnostic ====
        case ptDiagnostic:
            PillMgr.Write(PILL_I2C_ADDR, (PILL_START_ADDR + PILL_DOSETOP_ADDR), &Dose.Consts.Top, 4);
            break;

        default:
            Uart.Printf("Unknown Pill\r");
            rslt = FAILURE;
            break;
    } // switch
    SaveDoseToPill();   // Always save DoseAfter
    return rslt;
}

uint8_t App_t::IPillHandlerGrenade() {
    uint8_t rslt = OK;
    switch(Pill.Type) {
        case ptPanacea:
            Grenade.Charge = EMP_CHARGE_TOP;
            Grenade.State = gsReady;
            break;
        default:
            Uart.Printf("Unknown Pill\r");
            rslt = FAILURE;
            break;
    } // switch
    return rslt;
}

uint8_t App_t::IPillHandlerPillFlasher() {
    uint8_t rslt = FAILURE;
    // Write pill if data exists
    EE.ReadBuf(&Data2Wr, sizeof(Data2Wr), EE_REPDATA_ADDR);
    if(Data2Wr.Sz32 != 0) {
        Uart.Printf("#PillWrite32 0");
        for(uint8_t i=0; i<Data2Wr.Sz32; i++) Uart.Printf(",%d", Data2Wr.Data[i]);
        Uart.Printf("\r\n");
        rslt = PillMgr.Write(PILL_I2C_ADDR, PILL_START_ADDR, Data2Wr.Data, sizeof(Data2Wr));
        Uart.Ack(rslt);
    }
    return rslt;
}

// Everybody starts here
void App_t::OnPillConnect() {
    uint8_t rslt = PillMgr.Read(PILL_I2C_ADDR, PILL_START_ADDR, &Pill, sizeof(Pill_t));
    if(rslt == OK) {
        // Print pill
        Uart.Printf("#PillRead32 0 16\r\n");
        Uart.Printf("#PillData32 ");
        int32_t *p = (int32_t*)&Pill;
        for(uint32_t i=0; i<PILL_SZ32; i++) Uart.Printf("%d ", *p++);
        Uart.Printf("\r\n");
        // Everyone
        if(Pill.Type == ptSetType) ISetType(Pill.DeviceType);
        else {
            switch(Type) {
                case dtUmvos:       rslt = IPillHandlerUmvos();       break;
                case dtEmpGrenade:  rslt = IPillHandlerGrenade();     break;
                case dtPillFlasher: rslt = IPillHandlerPillFlasher(); break;
                default: break;
            }
        } // if set type
    } // if read ok
    // Indication
    if(rslt == OK) Indication.PillGood();
    else Indication.PillBad();
}
#endif

#if 1 // ==== Prolonged action Pill ====
void TmrProlongedPillCallback(void *p) {
    chSysLockFromIsr();
    chEvtSignalI(App.PThd, EVTMSK_PROLONGED_PILL);
    chSysUnlockFromIsr();
}

// Called first from pill handler, and then periodically by Timer+Event
uint8_t App_t::OnProlongedPill() {
    bool IsArmed;
    ProlongedState_t NewState = pstNothing; // Change indication state on completion
    uint8_t rslt = PillMgr.Read(PILL_I2C_ADDR, PILL_START_ADDR, &Pill, sizeof(Pill_t));
    if(rslt == OK) {
        switch(Pill.Type) {
            case ptAutodoc:
                if(Pill.ChargeCnt > 0) {
                    // Check if timer is armed and do nothing if yes (pill reconnected between tics)
                    chSysLock();
                    IsArmed = chVTIsArmedI(&TmrProlongedPill);
                    chSysUnlock();
                    if(!IsArmed) {
                        Pill.ChargeCnt--;
                        rslt = PillMgr.Write(PILL_I2C_ADDR, (PILL_START_ADDR + PILL_CHARGECNT_ADDR), &Pill.ChargeCnt, sizeof(Pill.ChargeCnt));
                        // Apply autodoc if not dead
                        if((rslt == OK) and (Dose.State != hsDeath)) {
                            Dose.Modify(Pill.Value);
                            // Check if healing completed
                            if(Dose.Value == 0) {
                                NewState = pstNothing;
                                Indication.AutodocCompleted();
                            }
                            else {
                                NewState = pstAutodoc;
                                // Start / restart prolonged timer
                                chVTSet(&TmrProlongedPill, MS2ST(T_PROLONGED_PILL_MS), TmrProlongedPillCallback, nullptr);
                            }
                        } // if rslt and !dead
                    } // if is armed
                } // if chargecnt
                else {
                    rslt = FAILURE;
                    // Indicate charge exhausted if previously was operational
                    if(Indication.ProlongedState == pstAutodoc) Indication.AutodocExhausted();
                }
                break;

            default: rslt = FAILURE; break;
        } // switch
        SaveDoseToPill();
    } // if ok
    Indication.ProlongedState = NewState; // Change indication state
    return rslt;
}
#endif
