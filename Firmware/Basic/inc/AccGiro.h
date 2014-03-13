/*
 * AccGiro.h
 *
 *  Created on: 16 февр. 2014 г.
 *      Author: Kreyl
 */

#ifndef ACCGIRO_H_
#define ACCGIRO_H_

#include "kl_lib_L15x.h"

// I2C & hardware
#define PERIPH_PWR_GPIO         GPIOB
#define PERIPH_PWR_PIN          7
#define AG_I2C_GPIO             GPIOB
#define AG_SCL_PIN              8
#define AG_SDA_PIN              9

#define AG_I2C                  I2C1
#define AG_I2C_BITRATE_HZ       200000
#define AG_DMATX                STM32_DMA1_STREAM6
#define AG_DMARX                STM32_DMA1_STREAM7

// ==== Accelerometer ====
#define ACC_ADDR                0x1C
// Registers addresses
#define ACC_REG_STATUS          0x00
#define ACC_REG_OUT_X_MSB       0x01
#define ACC_REG_XYZ_DATA_CFG    0x0E
#define ACC_FF_MT_CFG           0x15
#define ACC_FF_MT_SRC           0x16
#define ACC_FF_MT_THS           0x17
#define ACC_REG_CONTROL1        0x2A

// ==== Gyro ====
#define GYR_ADDR                0b1101001
// Registers addresses
#define GYR_WHO_AM_I            0x0F
#define GYR_CTRL_REG1           0x20
#define GYR_OUTX_L              0x28

class AccGyro_t {
private:
    i2c_t i2c;
    int16_t Convert(uint8_t HiByte, uint8_t LoByte);
public:
    int16_t a[3], w[3];
    void Init();
    void ReadAccelerations();
    void ReadSpeeds();
    //void StartMeasure
};

extern AccGyro_t AccGyro;

#endif /* ACCGIRO_H_ */
