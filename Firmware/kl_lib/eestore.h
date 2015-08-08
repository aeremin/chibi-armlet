/*
 * eestore.h
 *
 *  Created on: 14 но€б. 2013 г.
 *      Author: Kreyl
 */

#ifndef EESTORE_H_
#define EESTORE_H_

#include "kl_lib_L15x.h"
#include "cmd_uart.h"

/*
 * Data is stored repeatedly, thus EEPROM wearing is distributed.
 * Generally, data is stored as such: { Payload32, Counter16, EE_Sign16 }
 */
#define EE_STORE_SIGN   ((uint16_t)0x157A)  // == "ista", "know" on Quenya

struct EEUnit32_t {
    union {
        uint32_t dw32;
        struct {
            uint16_t Sign;
            uint16_t Counter;
        };
    };
    uint32_t Data;
    void Print() { Uart.Printf("\rcnt=%u; s=%X; d=%u", Counter, Sign, Data); }
} __attribute__ ((__packed__));
#define EE_UNIT_SZ     sizeof(EEUnit32_t)

#define EE_START_ADDR   128
#define EE_CNT          250
#define EE_PTR_FIRST    ((EEUnit32_t*)(EEPROM_BASE_ADDR + EE_START_ADDR))
#define EE_PTR_LAST     (EE_PTR_FIRST + EE_CNT - 1)

class EEStore_t : private Flash_t {
private:
    EEUnit32_t *PUnit = EE_PTR_FIRST;   // Place to write to
    uint8_t GetPrevUnit(EEUnit32_t** PPtr);
    uint8_t GetLastValuedPtr(EEUnit32_t** PPtr);
public:
    void Init();
    uint8_t Put(uint32_t AData);
    uint8_t Get(uint32_t *PData);
};

#endif /* EESTORE_H_ */
