/*
 * ee.h
 *
 *  Created on: 30 мая 2014 г.
 *      Author: g.kruglov
 */

#ifndef EE_H_
#define EE_H_

#if 1 // ==== Addresses ====
// Addresses
#define EE_DEVICE_ID_ADDR       0
#define EE_DEVICE_TYPE_ADDR     4
#define EE_DOSETOP_ADDR         8  // ID is uint32_t
#define EE_DRUG_TIMELEFT_ADDR   12
#define EE_DRUG_VALUE_ADDR      16
#define EE_EMP_STATE_ADDR       20
#define EE_REPDATA_ADDR         24 // Data storage for pill flasher

#endif

extern Eeprom_t EE;


#endif /* EE_H_ */
