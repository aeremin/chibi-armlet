/*
 * AccGyro.cpp
 *
 *  Created on: 16 февр. 2014 г.
 *      Author: Kreyl
 */

#include "AccGiro.h"
#include "cmd_uart.h"

AccGyro_t AccGyro;

void AccGyro_t::Init() {
    // Init pwr
    PinSetupOut(PERIPH_PWR_GPIO, PERIPH_PWR_PIN, omPushPull);   // Power
    PinSet(PERIPH_PWR_GPIO, PERIPH_PWR_PIN);
    chThdSleepMilliseconds(450);
    i2c.Init(AG_I2C, AG_I2C_GPIO, AG_SCL_PIN, AG_SDA_PIN, AG_I2C_BITRATE_HZ, AG_DMATX, AG_DMARX);

    uint8_t Buf[6];
    uint8_t Rslt;
    // ==== Accelerometer ====
    // Setup High-Pass filter and acceleration scale
    Buf[0] = ACC_REG_XYZ_DATA_CFG;
    Buf[1] = 0x01; // No filter, scale = 4g
    Rslt = i2c.CmdWriteWrite(ACC_ADDR, Buf, 2, nullptr, 0);
    if(Rslt) {
        Uart.Printf("AccErr1 %u\r", Rslt);
        return;
    }
    // Control registers
    Buf[0] = ACC_REG_CONTROL1; // CtrReg[0] is register address
    Buf[1] = 0x21;     // DR=100 => 50Hz data rate; Mode = Active
    Buf[2] = 0x00;     // Normal mode
    Buf[3] = 0x02;     // No IRQ; IRQ output active high
    Buf[4] = 0x00;     // All interrupts disabled
    Buf[5] = 0x04;     // FreeFall IRQ is routed to INT1 pin
    Rslt = i2c.CmdWriteWrite(ACC_ADDR, Buf, 6, nullptr, 0);
    if(Rslt) {
        Uart.Printf("AccErr2 %u\r", Rslt);
        return;
    }
    ReadAccelerations();

    // ==== Gyro ====
    // Control registers
    Buf[0] = GYR_CTRL_REG1 | 0x80; // register address with MSB set to autoincrease address
    Buf[1] = 0b00000111;    // Data Rate=100Hz, Bandwidth=12.5Hz, no PwrDown
    Buf[2] = 0x00;          // High Pass Filter disabled
    Buf[3] = 0x00;          // All interrupt signals disabled
    Buf[4] = 0x00;          // Cont.update, LSB@lowerAddr, FullScale=250dps
    Buf[5] = 0x00;          // No reboot, no FIFO, no HPF
    Rslt = i2c.CmdWriteWrite(ACC_ADDR, Buf, 6, nullptr, 0);
    if(Rslt) {
        Uart.Printf("AccErr1 %u\r", Rslt);
        return;
    }


//    Buf[0] = GYR_WHO_AM_I;
//    Rslt = i2c.CmdWriteRead(GYR_ADDR, Buf, 1, Buf, 1);
//    if(Rslt) {
//        Uart.Printf("GyrErr1 %u\r", Rslt);
//        return;
//    }
//    Uart.Printf("Gyr %u\r", Buf[0]);

}

int16_t AccGyro_t::Convert(uint8_t HiByte, uint8_t LoByte) {
    int16_t w;
    w = HiByte;
    w <<= 8;
    w |= LoByte;
    w /= 16;
    return w;
}

void AccGyro_t::ReadAccelerations() {
    uint8_t BufW[1], BufR[6];
    BufW[0] = ACC_REG_OUT_X_MSB;
    if(i2c.CmdWriteRead(ACC_ADDR, BufW, 1, BufR, 6) == OK) {
        // Convert received values to signed integers
        a[0] = Convert(BufR[0], BufR[1]);
        a[1] = Convert(BufR[2], BufR[3]);
        a[2] = Convert(BufR[4], BufR[5]);
    }
    else Uart.Printf("AccFail\r");
}

void AccGyro_t::ReadSpeeds() {
    uint8_t SpdAddr=(0x27 | 0x80), Buf[6];
    if(i2c.CmdWriteRead(GYR_ADDR, &SpdAddr, 1, Buf, 6) == OK) {
        Uart.Printf("%A\r", Buf, 6, ' ');
        // Convert received values to signed integers
        w[0] = Convert(Buf[0], Buf[1]);
        w[1] = Convert(Buf[2], Buf[3]);
        w[2] = Convert(Buf[4], Buf[5]);
    }
    else Uart.Printf("GyrFail\r");
}
