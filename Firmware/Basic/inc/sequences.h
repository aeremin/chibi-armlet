/*
 * sequences.h
 *
 *  Created on: Nov 9, 2013
 *      Author: kreyl
 */

#ifndef SEQUENCES_H_
#define SEQUENCES_H_

#include "peripheral.h"

#if 1 // ============================ LED RGB ==================================
const LedChunk_t ShortGreen[] = {
        {clGreen, 18, ckNormal},
        {clBlack, 990, ckLast},
};
#endif

#endif /* SEQUENCES_H_ */
