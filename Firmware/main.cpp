#include "kl_lib.h"
#include "ch.h"
#include "hal.h"
#include "uart.h"
#include "radio_lvl1.h"
//#include "vibro.h"
#include "led.h"
#include "Sequences.h"
#include "main.h"
#include "pill.h"
#include "pill_mgr.h"
#include "kl_i2c.h"

#if 1 // ======================== Variables and defines ========================
// Forever
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
static const UartParams_t CmdUartParams(115200, CMD_UART_PARAMS);
CmdUart_t Uart{&CmdUartParams};

uint8_t ID = 0;

LedRGB_t Led { LED_R_PIN, LED_G_PIN, LED_B_PIN };

cc1101_t CC(CC_Setup0);
#endif

int main(void) {
    Iwdg::InitAndStart(450);
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V5);
    Clk.SetMSI4MHz();
    Clk.UpdateFreqValues();
    // ==== Init OS ====
    halInit();
    chSysInit();

    // ==== Init Hard & Soft ====
    PinSetupOut(GPIOC, 15, omPushPull);
    PinSetHi(GPIOC, 15);
    PinSetupOut(GPIOC, 14, omPushPull);
    Uart.Init();
//    ReadIDfromEE();
//    Printf("\r%S %S\r", APP_NAME, XSTRINGIFY(BUILD_TIME));
//    Clk.PrintFreqs();

    if(!Sleep::WasInStandby()) Led.Init();

    if(CC.Init() == retvOk) {
        if(!Sleep::WasInStandby()) {
            Led.StartOrRestart(lsqStart);
            chThdSleepMilliseconds(360);
            Iwdg::Reload();
        }
        CC.SetTxPower(CC_PwrPlus5dBm);
//        CC.SetTxPower(CC_Pwr0dBm);

        CC.SetPktSize(RPKT_LEN);
        CC.SetChannel(0);
        rPkt_t Pkt;
        Pkt.ID = 11;
//        while(true) {
            PinSetHi(GPIOC, 14);
            CC.Recalibrate();
            CC.Transmit(&Pkt, RPKT_LEN);
            chThdSleepMilliseconds(2);
//            Iwdg::Reload();
//        }
//        PinSetLo(GPIOC, 14);
        //        uint8_t RxRslt = CC.Receive(RX_T_MS, &Pkt, RPKT_LEN, &Rssi);
        //        if(RxRslt == retvOk and Pkt.ID == ID) {
        //            Printf("%u: %d; %u\r", Pkt.ID, Rssi, Pkt.Cmd);
        CC.EnterPwrDown();
    }
    else {
        Led.Init();
        Led.StartOrRestart(lsqFailure);
        chThdSleepMilliseconds(999);
    }

    chSysLock();
    Iwdg::Reload();
    Sleep::EnterStandby();
    chSysUnlock();

    while(true); // Will never be here
}
