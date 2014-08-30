/*
 * led_rgb.h
 *
 *  Created on: 31 рту. 2014 у.
 *      Author: Kreyl
 */

#ifndef LED_RGB_H_
#define LED_RGB_H_

#include "hal.h"

// LED's ports, pins and Tmr
struct KeyData_t {
    GPIO_TypeDef *PGpio;
    uint16_t Pin;
};
const KeyData_t KeyData[] = {
        {GPIOD, 0},  // A
        {GPIOD, 1},  // B
        {GPIOD, 3},  // C
        {GPIOD, 9},  // X
        {GPIOD, 10}, // Y
        {GPIOD, 11}, // Z
        {GPIOD, 4},  // L
        {GPIOD, 7},  // E
        {GPIOD, 8},  // R
};



#endif /* LED_RGB_H_ */
