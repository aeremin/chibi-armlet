/*
 * peripheral.h
 *
 *  Created on: 18.01.2013
 *      Author: kreyl
 */

#ifndef PERIPHERAL_H_
#define PERIPHERAL_H_

#include <stdint.h>
#include "kl_lib_L15x.h"
#include "sequences.h"

#if 1 // ================================= Beep ================================
class Beeper_t {
private:
    VirtualTimer ITmr;
    const BeepChunk_t *IPFirstChunk;
public:
    void BeepI(const BeepChunk_t *PSequence);
    void Beep(const BeepChunk_t *PSequence) {   // Beep with this function
        IPFirstChunk = PSequence;
        chSysLock();
        BeepI(PSequence);
        chSysUnlock();
    }
    void Stop() {
        chSysLock();
        if(chVTIsArmedI(&ITmr)) chVTResetI(&ITmr);
        chSysUnlock();
        IPin.Set(0);
    }
    void Init();
    void Shutdown();
    // Inner use
    PwmPin_t IPin;
};
extern Beeper_t Beeper;
#endif

#define VIBRO_ENABLED   FALSE
#if VIBRO_ENABLED // ========================= Vibro ===========================
#define VIBRO_GPIO      GPIOB
#define VIBRO_PIN       6

enum StateOnOff_t {stOn, stOff};
struct VibroChunk_t {
    StateOnOff_t OnOff;
    uint16_t Time_ms;
    ChunkKind_t ChunkKind;
} PACKED;
#define VIBRO_CHUNK_SZ   sizeof(VibroChunk_t)

class Vibro_t {
private:
    VirtualTimer ITmr;
    const VibroChunk_t *IPFirstChunk;
public:
    void Flinch(const VibroChunk_t *PSequence) {   // Beep with this function
        IPFirstChunk = PSequence;
        chSysLock();
        IFlinchI(PSequence);
        chSysUnlock();
    }
    void Stop() {
        chSysLock();
        if(chVTIsArmedI(&ITmr)) chVTResetI(&ITmr);
        chSysUnlock();
        PinClear(VIBRO_GPIO, VIBRO_PIN);
    }
    void Init();
    // Inner use
    void IFlinchI(const VibroChunk_t *PSequence);
};
extern Vibro_t Vibro;
#endif

#endif /* PERIPHERAL_H_ */
