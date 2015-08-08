/*
 * eestore.cpp
 *
 *  Created on: 14 но€б. 2013 г.
 *      Author: Kreyl
 */

#include "eestore.h"

void EEStore_t::Init() {
    if(GetLastValuedPtr(&PUnit) == OK) {
        PUnit++;
        if(PUnit > EE_PTR_LAST) PUnit = EE_PTR_FIRST;
    }
    else PUnit = EE_PTR_FIRST;
}

uint8_t EEStore_t::GetLastValuedPtr(EEUnit32_t** PPtr) {
    EEUnit32_t *Arr = EE_PTR_FIRST;
    // Check if anything is stored at beginning
    if(Arr[0].Sign != EE_STORE_SIGN) return FAILURE;
    // Iterate through array
    for(uint32_t i=1; i < EE_CNT; i++) {
        // Check if nothing or data tail
        Arr[i].Print();
        Arr[i-1].Print();
        Uart.Printf("\r dif=%u", (uint16_t)(Arr[i].Counter - Arr[i-1].Counter));
        if((Arr[i].Sign != EE_STORE_SIGN) or (uint8_t)(Arr[i].Counter - Arr[i-1].Counter) != 1) {
            *PPtr = &Arr[i-1];
            return OK;
        }
    }
    // Will be here if counter diff was 1 all the way. Return Last value.
    *PPtr = &Arr[EE_CNT-1];
    return OK;
}

uint8_t EEStore_t::Put(uint32_t AData) {
    // Prepare unit to write
    EEUnit32_t NewU, *pprev;
    NewU.Data = AData;
    NewU.Sign = EE_STORE_SIGN;
    if(GetPrevUnit(&pprev) == OK) NewU.Counter = pprev->Counter + 1;
    else NewU.Counter = 0;

    // Write new unit
    volatile uint32_t *PDst32 = (uint32_t*)PUnit;
    uint32_t *PSrc32 = (uint32_t*)&NewU;
    UnlockEE();
    chSysLock();
    // Wait for last operation to be completed
    uint8_t r = WaitForLastOperation();
    if(r == OK) {
        *PDst32++ = *PSrc32++;
        r = WaitForLastOperation();
        if(r == OK) {
            *PDst32 = *PSrc32;
            r = WaitForLastOperation();
        }
    }
    LockEE();
    chSysUnlock();

    // Increase pointer
    if(r == OK) {
        PUnit++;
        if(PUnit > EE_PTR_LAST) PUnit = EE_PTR_FIRST;
    }
    return r;
}

uint8_t EEStore_t::Get(uint32_t *PData) {
    EEUnit32_t *pprev;
    if(GetPrevUnit(&pprev) != OK) return FAILURE;
    else {
        *PData = pprev->Data;
        return OK;
    }
}

uint8_t EEStore_t::GetPrevUnit(EEUnit32_t** PPtr) {
    if(PUnit == EE_PTR_FIRST) *PPtr = EE_PTR_LAST;
    else *PPtr = PUnit - 1;
    return ((*PPtr)->Sign == EE_STORE_SIGN)? OK : FAILURE;
}
