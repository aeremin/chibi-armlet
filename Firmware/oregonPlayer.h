/*****************************************************************************
* Model: oregonPlayer.qm
* File:  ./oregonPlayer.h
*
* This code has been generated by QM tool (see state-machine.com/qm).
* DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*****************************************************************************/
/*${.::oregonPlayer.h} .....................................................*/
#ifndef oregonPlayer_h
#define oregonPlayer_h
#ifdef __cplusplus
extern "C" {
#endif
#include "qhsm.h"    /* include own framework tagunil version */

#define HEALTHY 0
#define AGONY 1
#define DEAD 2
#define GHOUL_GOOD 3
#define GHOUL_WOUNDED 4
#define GHOUL_HEALING 5
#define BLESSED 6
#define FLASH_MS 300
#define FLASH_1M 60100
#define TIMEOUT_AGONY_S 600
#define TIMEOUT_DEATH_S 15
#define TIMEOUT_RADX_S 900
#define LONG_BEEP_MS 15000
#define MEDIUM_BEEP_MS 3000
#define SHORT_BEEP_MS 300
#define DEFAULT_HP 27000
#define GHOUL_HP (DEFAULT_HP/3)

/*${SMs::OregonPlayer} .....................................................*/
typedef struct {
/* protected: */
    QHsm super;

/* public: */
    unsigned int CharHP;
    QStateHandler StartState;
    unsigned int TimerAgony;
    unsigned int TimerDeath;
} OregonPlayer;

/* protected: */


typedef struct oregonPlayerQEvt {
    QEvt super;
    unsigned int value;
} oregonPlayerQEvt;

enum PlayerSignals {
TICK_SEC_SIG = Q_USER_SIG,

RAD_RCVD_SIG,
HEAL_SIG,

TIME_TICK_1S_SIG,
TIME_TICK_10S_SIG,
TIME_TICK_1M_SIG,

PILL_ANY_SIG,
PILL_HEAL_SIG,
PILL_HEALSTATION_SIG,
PILL_BLESS_SIG,
PILL_CURSE_SIG,
PILL_ANTIRAD_SIG,
PILL_RAD_X_SIG,
PILL_GHOUL_SIG,
PILL_GHOUL_REMOVED_SIG,
PILL_REMOVED_SIG,
PILL_RESET_SIG,
PILL_TEST_SIG,

AGONY_SIG,
IMMUNE_SIG,
NOT_IMMUNE_SIG,
BLESSED_SIG,

LAST_USER_SIG
};
extern QHsm * const the_oregonPlayer; /* opaque pointer to the oregonPlayer HSM */

/*${SMs::OregonPlayer_ctor} ................................................*/
void OregonPlayer_ctor(
    unsigned int HP,
    unsigned int State,
    unsigned int TimerAgony);

#ifdef __cplusplus
}
#endif
#endif /* oregonPlayer_h */
