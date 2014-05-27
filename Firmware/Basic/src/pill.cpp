/*
 * pill.cpp
 *
 *  Created on: 22 ��� 2014 �.
 *      Author: g.kruglov
 */

#include "application.h"
#include "pill_mgr.h"

#if 1 // ================================ On Connect ===========================
void App_t::IPillHandlerUmvos() {
    uint8_t rslt = OK;
    switch(Pill.Type) {

        // ==== Cure ====
        case ptCure:
            if(Pill.ChargeCnt > 0) {    // Check charge count, decrease it and write it back
                Pill.ChargeCnt--;
                rslt = PillMgr.Write(PILL_I2C_ADDR, (PILL_START_ADDR + PILL_CHARGECNT_ADDR), &Pill.ChargeCnt, sizeof(Pill.ChargeCnt));
                if((rslt == OK) and (Dose.State != hsDeath)) {    // Modify dose if not dead
                    Dose.Modify(Pill.Value, diNeverIndicate);
                }
            }
            else rslt = FAILURE;
            break;

        // ==== Drug ====
        case ptDrug:
            if(Pill.ChargeCnt > 0) {
                Pill.ChargeCnt--;
                rslt = PillMgr.Write(PILL_I2C_ADDR, (PILL_START_ADDR + PILL_CHARGECNT_ADDR), &Pill.ChargeCnt, sizeof(Pill.ChargeCnt));
                if(rslt == OK) {
                    if(Dose.State != hsDeath) Dose.Drug.Set(Pill.Value, Pill.Time_s);
                }
            }
            else rslt = FAILURE;
            break;

        // ==== Panacea ====
        case ptPanacea:
            Dose.Reset();
            Dose.Drug.Set(0, 0); // Reset drug
            break;

        // ==== Set DoseTop ====
        case ptSetDoseTop:
            Dose.Consts.Setup(Pill.DoseTop);
            SaveDoseTop();
            Uart.Printf("Top=%u; Red=%u; Yellow=%u\r", Dose.Consts.Top, Dose.Consts.Red, Dose.Consts.Yellow);
            break;

        // ==== Diagnostic ====
        case ptDiagnostic: PillMgr.Write(PILL_I2C_ADDR, (PILL_START_ADDR + PILL_DOSETOP_ADDR), &Dose.Consts.Top, 4); break;

        default:
            Uart.Printf("Unknown Pill\r");
            rslt = FAILURE;
            break;
    } // switch
    // Save DoseAfter
    PillMgr.Write(PILL_I2C_ADDR, (PILL_START_ADDR + PILL_DOSEAFTER_ADDR), &Dose.Value, 4);
    // ==== Indication ====
    if(rslt == OK) {
        Beeper.Beep(BeepPillOk);
        Led.StartBlink(LedPillCureOk);
    }
    else {
        Beeper.Beep(BeepPillBad);
        Led.StartBlink(LedPillBad);
    }
    chThdSleepMilliseconds(1008);   // Let indication to complete
    Dose.RenewIndication();
}

void App_t::IPillHandlerPillFlasher() {
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

// Everybody starts here
void App_t::OnPillConnect() {
    if(PillMgr.Read(PILL_I2C_ADDR, PILL_START_ADDR, &Pill, sizeof(Pill_t)) != OK) return;
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
            case dtUmvos:       IPillHandlerUmvos();       break;
            case dtPillFlasher: IPillHandlerPillFlasher(); break;
            default: break;
        }
    } // if set type
}
#endif

#if 1 // ============================= On Disconnect ===========================
void App_t::OnPillDisconnect() {

}
#endif
