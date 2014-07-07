/*
 * File:   main.cpp
 * Author: Kreyl
 * Project: Armlet2South
 *
 * Created on Feb 05, 2013, 20:27
 */

#include "kl_lib_L15x.h"
#include "ch.h"
#include "hal.h"
#include "clocking_L1xx.h"
#include "cmd_uart.h"
#include "evt_mask.h"
#include "cc1101defins.h"
#include "rlvl1_defins.h"
#include "cc1101.h"
#include "L3G.h"
#include "lsm303.h"

// Device type
#define TRANSMITTER
#ifndef TRANSMITTER
#define RECEIVER
#endif

// I2C & hardware
#define PERIPH_PWR_GPIO     GPIOB
#define PERIPH_PWR_PIN      7
#define I2C_GPIO            GPIOB
#define SCL_PIN             8
#define SDA_PIN             9

#define I2C                 I2C1
#define I2C_BITRATE_HZ      200000
#define I2C_DMATX           STM32_DMA1_STREAM6
#define I2C_DMARX           STM32_DMA1_STREAM7

i2c_t i2c;
L3G gyro;
LSM303 compass;

int main(void) {
    // ==== Init Vcore & clock system ====
    SetupVCore(vcore1V2);
    Clk.UpdateFreqValues();
    // ==== Init OS ====
    halInit();
    chSysInit();

    chThdSleepMilliseconds(999);

    // ==== Init Hard & Soft ====
    Uart.Init(115200);
    Uart.Printf("\rRst\r\n");

    // Init radioIC
    CC.Init();
    CC.SetTxPower(CC_Pwr0dBm);
    CC.SetPktSize(RPKT_LEN);
    CC.SetChannel(4);

#ifdef TRANSMITTER
    // I2C
    i2c.Standby();
    i2c.Init(I2C, I2C_GPIO, SCL_PIN, SDA_PIN, I2C_BITRATE_HZ, I2C_DMATX, I2C_DMARX);

    // Sensors
    gyro.init();
    gyro.writeReg(L3G_CTRL_REG4, 0x20); // 2000 dps full scale
    gyro.writeReg(L3G_CTRL_REG1, 0x0F); // normal power mode, all axes enabled, 100 Hz

    compass.init();
    compass.enableDefault();
    compass.writeReg(LSM303::CTRL_REG4_A, 0x28); // 8 g full scale: FS = 10; high resolution output mode
#endif

    rPkt_t rPkt;
    uint8_t *p = (uint8_t*)&rPkt;
    for(uint8_t i=0; i<sizeof(rPkt_t); i++) *p++ = 0;

    while(true) {
#ifdef TRANSMITTER
        chThdSleepMilliseconds(2);
        gyro.read(&rPkt.AngVelU, &rPkt.AngVelV, &rPkt.AngVelW);
        compass.read();
        rPkt.AccX = compass.a.x;
        rPkt.AccY = compass.a.y;
        rPkt.AccZ = compass.a.z;
        rPkt.AngleU = compass.m.x;
        rPkt.AngleV = compass.m.y;
        rPkt.AngleW = compass.m.z;

//        Uart.Printf(
//                "%d %d %d %d %d %d %d %d %d\r\n",
//                rPkt.AccX, rPkt.AccY, rPkt.AccZ,
//                rPkt.AngleU, rPkt.AngleV, rPkt.AngleW,
//                rPkt.AngVelU, rPkt.AngVelV, rPkt.AngVelW
//        );
        CC.TransmitSync((void*)&rPkt);
#else
        int8_t Rssi;
        uint8_t RxRslt = CC.ReceiveSync(36, &rPkt, &Rssi);
        if(RxRslt == OK) {
            Uart.Printf(
                    "%d %d %d %d %d %d %d %d %d\r\n",
                    rPkt.AccX, rPkt.AccY, rPkt.AccZ,
                    rPkt.AngleU, rPkt.AngleV, rPkt.AngleW,
                    rPkt.AngVelU, rPkt.AngVelV, rPkt.AngVelW
            );
        }
#endif
    } // while true
}
