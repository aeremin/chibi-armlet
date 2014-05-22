/*
 * pill.h
 *
 *  Created on: 22 мая 2014 г.
 *      Author: g.kruglov
 */

#ifndef PILL_H_
#define PILL_H_

enum PillType_t {
    ptSetID = 1,
    ptCure = 9,
    ptDrug = 10,
    ptPanacea = 11,
    ptAutodoc = 12,
    ptSetDoseTop = 18,
    ptDiagnostic = 27,
    ptEmpBreaker = 31,
    ptEmpRepair = 32,
    ptElectrostation = 33,
    ptBattery = 34,
    ptSetType = 99,
};

struct Pill_t {
    union {
        int32_t TypeInt32;
        PillType_t Type;
    };
    union {
        int32_t DeviceID;
        int32_t DoseAfter;  // Contains dose value after pill application
    };
    union {
        // Cure / drug
        struct {
            int32_t ChargeCnt;
            int32_t Value;
        } __attribute__ ((__packed__));
        int32_t DoseTop;
    };
} __attribute__ ((__packed__));
#define PILL_SZ     sizeof(Pill_t)
#define PILL_SZ32   (sizeof(Pill_t) / sizeof(int32_t))

// Data to save in EE and to write to pill
struct DataToWrite_t {
    uint32_t Sz32;
    int32_t Data[PILL_SZ32];
};

#endif /* PILL_H_ */
