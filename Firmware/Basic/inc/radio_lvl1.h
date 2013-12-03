/*
 * radio_lvl1.h
 *
 *  Created on: Nov 17, 2013
 *      Author: kreyl
 */

#ifndef RADIO_LVL1_H_
#define RADIO_LVL1_H_

#include "ch.h"
#include "rlvl1_defins.h"

class rLevel1_t {
private:
    rPkt_t PktRx;       // Local rPkt to receive
    rPkt_t PktTx;       // FIXME: remove if not needed
    Thread *PThread;
    uint32_t Damage;
    bool NewDamageReady;
public:
    void Init();
    uint32_t GetDamage();
    // Inner use
    void ITask();
    void IDamageTmrCallback();
};

extern rLevel1_t Radio;

#endif /* RADIO_LVL1_H_ */
