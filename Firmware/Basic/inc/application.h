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
    uint16_t ID;
    void Init();
};

extern App_t App;

#endif /* APPLICATION_H_ */
