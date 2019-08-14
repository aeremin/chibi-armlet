#include "kl_lib.h"
#include "ch.h"
#include "hal.h"
#include "uart.h"
#include "radio_lvl1.h"
#include "beeper.h"
#include "led.h"
#include "Sequences.h"

#if 1 // ======================== Variables and defines ========================
// Forever
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
static const UartParams_t CmdUartParams(115200, CMD_UART_PARAMS);
CmdUart_t Uart{&CmdUartParams};

__unused
static const uint8_t PwrTable[12] = {
        CC_PwrMinus30dBm, // 0
        CC_PwrMinus27dBm, // 1
        CC_PwrMinus25dBm, // 2
        CC_PwrMinus20dBm, // 3
        CC_PwrMinus15dBm, // 4
        CC_PwrMinus10dBm, // 5
        CC_PwrMinus6dBm,  // 6
        CC_Pwr0dBm,       // 7
        CC_PwrPlus5dBm,   // 8
        CC_PwrPlus7dBm,   // 9
        CC_PwrPlus10dBm,  // 10
        CC_PwrPlus12dBm   // 11
};

rPkt_t Pkt;

#define MESH_PWR_LVL_ID     8   // 5 dBm
#define DEEPSLEEP_TIME_MS   153 // time between activity
#define MESH_RX_TIME_MS     36  // For how long to RX waiting valid pkt
#define MESH_TX_TIME_MS     180 // For how long to transmit what need to be transmitted
#define MESH_DELAY_BETWEEN_RETRANSMIT_MS_MIN    11
#define MESH_DELAY_BETWEEN_RETRANSMIT_MS_MAX    36
#define MESH_RETRANSMIT_COUNT                   9

int32_t ID = 1;
int32_t RssiThr = -99;
uint8_t PwrLvlId = 7; // 0dBm
uint8_t Damage = 1;
#define ID_NO_BEACON        1500

// EEAddresses
#define EE_ADDR_ID          0
#define EE_ADDR_THRESHOLD   4
#define EE_ADDR_PWR_LVL_ID  8
#define EE_ADDR_DAMAGE_ID   12

LedRGB_t Led { LED_R_PIN, LED_G_PIN, LED_B_PIN };
Beeper_t Beeper {BEEPER_PIN};

cc1101_t CC(CC_Setup0);

void OnCmd(Cmd_t *PCmd);

void ReadParams();
uint8_t SetID(int32_t NewID);
uint8_t SetThr(int32_t NewThr);
uint8_t SetPwr(uint32_t NewPwrId);
uint8_t SetDmg(uint32_t NewDmg);

void MeshTask();
void BeaconTask();
void ProcessRCmdAndPrepareReply();
void TransmitMeshPkt();
#endif

#if 1 // ================= RingBuf for PktIDs =================
#define PKTID_BUF_SZ    12
class IdAndTmr_t {
public:
    uint32_t Tmr = 0;
    uint8_t ID = 0;
} __packed;

class CircBufId_t {
private:
    IdAndTmr_t IBuf[PKTID_BUF_SZ];
public:
    bool Presents(uint8_t AID) {
        for(IdAndTmr_t &IIdTmr : IBuf) {
            if(IIdTmr.ID == AID) return true;
        }
        return false;
    }

    void Add(uint8_t AID) {
        uint32_t OldestIndx = 0, TmrMax  = 0;
        for(uint32_t i=0; i<PKTID_BUF_SZ; i++) {
            if(IBuf[i].ID == 0) { // Fill if empty
                IBuf[i].ID = AID;
                IBuf[i].Tmr = 0;
                return;
            }
            else { // Find oldest ID
                if(IBuf[i].Tmr >= TmrMax) {
                    TmrMax = IBuf[i].Tmr;
                    OldestIndx = i;
                }
            }
        } // for
        // Fill oldest value with new one
        IBuf[OldestIndx].ID = AID;
        IBuf[OldestIndx].Tmr = 0;
    }

    void Clear() {
        for(uint32_t i=0; i<PKTID_BUF_SZ; i++) {
            IBuf[i].ID = 0;
            IBuf[i].Tmr = 0;
        }
    }

    void CheckAndFlushWhatNeeded() {}

    void Load() {}
    void Save() {}
};

CircBufId_t PktIdBuf;
#endif

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V5);
    Clk.SetMSI4MHz();
    Clk.UpdateFreqValues();
    // ==== Init OS ====
    halInit();
    chSysInit();

    // ==== Init Hard & Soft ====
    PinSetupOut(GPIOC, 15, omPushPull);
    PinSetHi(GPIOC, 15);                // To measure ON time
    PinSetupOut(GPIOC, 14, omPushPull); // To measure TX time
    Uart.Init();
    ReadParams();
    if(Sleep::WasInStandby()) {
        PktIdBuf.Load();
    }
    else {
        Led.Init();
        Led.StartOrRestart(lsqStart0);
        Printf("\r%S %S\rID=%u; Thr=%d; Pwr=%u\r", APP_NAME, XSTRINGIFY(BUILD_TIME), ID, RssiThr, PwrLvlId);
        Clk.PrintFreqs();
        PktIdBuf.Clear();

        // Try to receive Cmd by UART
        for(int i=0; i<27; i++) {
            chThdSleepMilliseconds(99);
            uint8_t b;
            while(Uart.GetByte(&b) == retvOk) {
                if(Uart.Cmd.PutChar(b) == pdrNewCmd) {
                    OnCmd(&Uart.Cmd);
                    i = 0;
                }
            } // while get byte
        } // for
    }

    if(CC.Init() == retvOk) {
        if(!Sleep::WasInStandby()) { // Show CC is ok
            Led.StartOrRestart(lsqStart);
            chThdSleepMilliseconds(999);
        }
        // Common CC params
        CC.SetPktSize(RPKT_LEN);
        CC.SetChannel(0);
        // Tasks
        MeshTask();
        if(ID < ID_NO_BEACON) BeaconTask();
    }
    else { // CC failure
        Led.Init();
        Led.StartOrRestart(lsqFailure);
        chThdSleepMilliseconds(999);
    }

    // Enter sleep
    CC.EnterPwrDown();
    chSysLock();
    Iwdg::InitAndStart(DEEPSLEEP_TIME_MS);
    Sleep::EnterStandby();
    chSysUnlock();

    while(true); // Will never be here
}

void MeshTask() {
    // Process buffer
    PktIdBuf.CheckAndFlushWhatNeeded();
    // Rx for some time
    systime_t Start = chVTGetSystemTimeX();
    while(true) {
        int32_t RxTime_ms = (int32_t)MESH_RX_TIME_MS - (int32_t)(TIME_I2MS(chVTTimeElapsedSinceX(Start)));
        if(RxTime_ms <= 0) break;
        if(CC.Receive(RxTime_ms, &Pkt, RPKT_LEN) == retvOk) {
//            Printf("%u: Thr: %d; Pwr: %u\r", Pkt.From, Pkt.RssiThr, Pkt.PowerLvlId);
//            chThdSleepMilliseconds(9);
            if(Pkt.To == ID) { // For us!
                ProcessRCmdAndPrepareReply();
                TransmitMeshPkt();
                break;
            }
            else { // For someone else
                if(Pkt.PktID != 0) { // Needs retransmission
                    if(!PktIdBuf.Presents(Pkt.PktID)) { // Was not retransmitted yet
                        PktIdBuf.Add(Pkt.PktID);
                        PktIdBuf.Save();
                        TransmitMeshPkt();
                        break;
                    } // if was not retransmitted
                } // if needs retransmission
            } // if for us
        } // if RX ok
    } // while
}

// Process Cmd-for-us
void ProcessRCmdAndPrepareReply() {
    // Common preparations
    Pkt.To = Pkt.From;
    Pkt.From = ID;
    if(Pkt.PktID != 0) { // Increase PktID if not 0
        if(Pkt.PktID >= PKTID_TOP_VALUE) Pkt.PktID = 1;
        else Pkt.PktID++;
    }
    Pkt.Cmd = rcmdPong;
    Pkt.Pong.MaxLvlID = Pkt.TransmitterID;
    Pkt.Pong.Reply = retvOk;
    Pkt.TransmitterID = ID;

    // Individual preparations
    switch(Pkt.Cmd) {
        case rcmdPing: break; // Nothing here, just do not fall into default

        case rcmdScream:
            Led.Init();
            Beeper.Init();
            Beeper.StartOrRestart(bsqSearch);
            Led.StartOrRestart(lsqSearch);
            chThdSleepMilliseconds(108);
            break;

        case rcmdLustraParams:
            if(SetPwr(Pkt.LustraParams.Power) != retvOk) Pkt.Pong.Reply = retvBadValue;
            else if(SetThr(Pkt.LustraParams.RssiThr) != retvOk) Pkt.Pong.Reply = retvBadValue;
            else if(SetDmg(Pkt.LustraParams.Damage) != retvOk) Pkt.Pong.Reply = retvBadValue;
            break;

        default: Pkt.Pong.Reply = retvCmdError; break;
    } // switch
}

void TransmitMeshPkt() {
//    CC.SetTxPower(PwrTable[MESH_PWR_LVL_ID]);
}

void BeaconTask() {
    CC.SetTxPower(PwrTable[PwrLvlId]);
    Pkt.From = ID;
    Pkt.TransmitterID = ID;
    Pkt.Cmd = rcmdBeacon;
    Pkt.PktID = PKTID_DO_NOT_RETRANSMIT;
    Pkt.Beacon.RssiThr = RssiThr;
    Pkt.Beacon.Damage = Damage;
    PinSetHi(GPIOC, 14); // DEBUG
    CC.Recalibrate();
    CC.Transmit(&Pkt, RPKT_LEN);
    PinSetLo(GPIOC, 14); // DEBUG
}

#if 1 // =========================== Cmd handling ==============================
void Ack(int32_t Result) { Printf("Ack %d\r\n", Result); }

void OnCmd(Cmd_t *PCmd) {
    if(PCmd->NameIs("Ping")) {
        Ack(retvOk);
    }
    else if(PCmd->NameIs("GetID")) Printf("ID %u\r", ID);

    else if(PCmd->NameIs("SetID")) {
        int32_t NewID;
        if(PCmd->GetNext<int32_t>(&NewID) != retvOk) { Ack(retvCmdError); return; }
        Ack(SetID(NewID));
    }

    else if(PCmd->NameIs("GetThr")) Printf("Thr %d\r", RssiThr);

    else if(PCmd->NameIs("SetThr")) {
        int32_t NewThr;
        if(PCmd->GetNext<int32_t>(&NewThr) != retvOk) { Ack(retvCmdError); return; }
        Ack(SetThr(NewThr));
    }

    else if(PCmd->NameIs("SetPwr")) {
        uint32_t NewPwr;
        if(PCmd->GetNext<uint32_t>(&NewPwr) != retvOk) { Ack(retvCmdError); return; }
        if(NewPwr > 11) NewPwr = 11;
        Ack(SetPwr(NewPwr));
    }

    else if(PCmd->NameIs("Set")) {
        int32_t NewID;
        if(PCmd->GetNext<int32_t>(&NewID) != retvOk) { Ack(retvCmdError); return; }
        int32_t NewThr;
        if(PCmd->GetNext<int32_t>(&NewThr) != retvOk) { Ack(retvCmdError); return; }
        uint32_t NewPwr;
        if(PCmd->GetNext<uint32_t>(&NewPwr) != retvOk) { Ack(retvCmdError); return; }
        if(NewPwr > 11) NewPwr = 11;
        Printf("Ack %u %u %u\r\n", SetID(NewID), SetThr(NewThr), SetPwr(NewPwr));
    }
}
#endif

#if 1 // ============================ Save/Load Params =========================
void ReadParams() {
    ID = EE::Read32(EE_ADDR_ID);
    RssiThr = EE::Read32(EE_ADDR_THRESHOLD);
    PwrLvlId = EE::Read32(EE_ADDR_PWR_LVL_ID);
    if(PwrLvlId > 11) PwrLvlId = 11;
}

uint8_t SetID(int32_t NewID) {
    uint8_t rslt = EE::Write32(EE_ADDR_ID, NewID);
    if(rslt == retvOk) {
        ID = NewID;
        return retvOk;
    }
    else {
        Printf("EE error: %u\r", rslt);
        return retvFail;
    }
}

uint8_t SetThr(int32_t NewThr) {
    uint8_t rslt = EE::Write32(EE_ADDR_THRESHOLD, NewThr);
    if(rslt == retvOk) {
        RssiThr = NewThr;
        return retvOk;
    }
    else {
        Printf("EE error: %u\r", rslt);
        return retvFail;
    }
}

uint8_t SetPwr(uint32_t NewPwrId) {
    uint8_t rslt = EE::Write32(EE_ADDR_PWR_LVL_ID, NewPwrId);
    if(rslt == retvOk) {
        PwrLvlId = NewPwrId;
        return retvOk;
    }
    else {
        Printf("EE error: %u\r", rslt);
        return retvFail;
    }
}
#endif
