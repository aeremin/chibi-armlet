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
    Indication.HealthRenew();
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

#if 1 // ============================= On Disconnect ===========================
void App_t::OnPillDisconnect() {

}
#endif
