/*
 * pill.cpp
 *
 *  Created on: 22 мая 2014 г.
 *      Author: g.kruglov
 */

#include "application.h"
#include "pill_mgr.h"

#if 1 // ================================ On Connect ===========================
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
        switch(Pill.Type) {
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
            case ptCure:
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

            // ==== Set DoseTop ====
            case ptSetDoseTop:
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
