/*
 * calcs.cpp
 *
 *  Created on: 20 июля 2014 г.
 *      Author: Kreyl
 */

#include "calcs.h"
#include "ch.h"

Calculations_t Calc;

void Calculations_t::TimeSSinceLastTime() {
    timer_old = timer;
    timer = chTimeNow();
    G_Dt = (timer - timer_old) / 1000.0;
}
