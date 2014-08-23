/*
 * calcs.h
 *
 *  Created on: 20 июля 2014 г.
 *      Author: Kreyl
 */

#ifndef CALCS_H_
#define CALCS_H_

class Calculations_t {
private:

public:
    uint32_t timer, timer_old;
    float G_Dt; // Integration time (DCM algorithm) We will run the integration loop at 50Hz if possible
    void TimeSSinceLastTime();
};

extern Calculations_t Calc;

#endif /* CALCS_H_ */
