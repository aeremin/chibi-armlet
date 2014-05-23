/*
 * evt_mask.h
 *
 *  Created on: Apr 12, 2013
 *      Author: g.kruglov
 */

#ifndef EVT_MASK_H_
#define EVT_MASK_H_

// Event masks
#define EVTMSK_CLICK            EVENT_MASK(0)
#define EVTMSK_DOSE_STORE       EVENT_MASK(1)
#define EVTMSK_PILL_CHECK       EVENT_MASK(2)
#define EVTMSK_RX_TABLE_READY   EVENT_MASK(3)
#define EVTMSK_MEASURE_TIME     EVENT_MASK(4)
#define EVTMSK_MEASUREMENT_DONE EVENT_MASK(5)
#define EVTMSK_UART_RX_POLL     EVENT_MASK(6)
#define EVTMSK_PELENG_FOUND     EVENT_MASK(7)

#endif /* EVT_MASK_H_ */
