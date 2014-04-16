/*
 * evt_mask.h
 *
 *  Created on: Apr 12, 2013
 *      Author: g.kruglov
 */

#ifndef EVT_MASK_H_
#define EVT_MASK_H_

// Event masks
#define EVTMSK_DOSE_INC             EVENT_MASK(0)
#define EVTMSK_DOSE_STORE           EVENT_MASK(1)
#define EVTMSK_PILL_CHECK           EVENT_MASK(2)
#define EVTMASK_RADIO_RX            EVENT_MASK(3)


#define EVTMSK_SENS_TABLE_READY     EVENT_MASK(8)
#define EVTMSK_NEW_CYCLE            EVENT_MASK(9)
#define EVTMSK_UPDATE_CYCLE         EVENT_MASK(10)
#define EVTMSK_MESH_RX              EVENT_MASK(11)
#define EVTMSK_MESH_TX              EVENT_MASK(12)
#define EVTMSK_LED_UPD              EVENT_MASK(13)

#define EVT_MSK_BKT_EXTRACT          EVENT_MASK(14)

#endif /* EVT_MASK_H_ */
