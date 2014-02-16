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
    // ==== Accelerometer ====
    // Setup initial registers
    uint8_t BufW[6];
    uint8_t Rslt;
    // Setup High-Pass filter and acceleration scale
    BufW[0] = ACC_REG_XYZ_DATA_CFG;
    BufW[1] = 0x01; // No filter, scale = 4g
    Rslt = i2c.CmdWriteWrite(ACC_ADDR, BufW, 2, nullptr, 0);
    if(Rslt) {
        Uart.Printf("AccErr1 %u\r", Rslt);
        return;
    }
    // Control registers
    BufW[0] = ACC_REG_CONTROL1; // CtrReg[0] is register address
    BufW[1] = 0x21;     // DR=100 => 50Hz data rate; Mode = Active
    BufW[2] = 0x00;     // Normal mode
    BufW[3] = 0x02;     // No IRQ; IRQ output active high
    BufW[4] = 0x00;     // All interrupts disabled
    BufW[5] = 0x04;     // FreeFall IRQ is routed to INT1 pin
    Rslt = i2c.CmdWriteWrite(ACC_ADDR, BufW, 6, nullptr, 0);
    if(Rslt) {
        Uart.Printf("AccErr2 %u\r", Rslt);
        return;
    }
    ReadAccelerations();
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
