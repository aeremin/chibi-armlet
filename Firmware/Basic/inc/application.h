/*
 * application.h
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "kl_lib_L15x.h"

class App_t {
private:
public:
    Thread *PThd;
    VirtualTimer TmrUartRx;
    // Events
//    void OnUartCmd(Cmd_t *PCmd);
};

extern App_t App;

#endif /* APPLICATION_H_ */
