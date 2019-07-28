/*
 * Sequences.h
 *
 *  Created on: 09 џэт. 2015 у.
 *      Author: Kreyl
 */

#pragma once

#include "ChunkTypes.h"
#include "main.h"

#if 1 // ============================ LED RGB ==================================
const LedRGBChunk_t lsqStart[] = {
        {csSetup, 360, clGreen},
        {csWait, 270},
#if GREEN_AND_WHITE
        {csSetup, 360, clWhite},
        {csWait, 270},
#endif
        {csSetup, 360, clBlack},
        {csEnd},
};

const LedRGBChunk_t lsqFailure[] = {
        {csSetup, 0, clRed},
        {csWait, 99},
        {csSetup, 0, clBlack},
        {csWait, 99},
        {csSetup, 0, clRed},
        {csWait, 99},
        {csSetup, 0, clBlack},
        {csWait, 99},
        {csSetup, 0, clRed},
        {csWait, 99},
        {csSetup, 0, clBlack},
        {csEnd}
};

const LedRGBChunk_t lsqDisappear[] = {
        {csSetup, 360, clBlack},
        {csEnd}
};

const LedRGBChunk_t lsqAppearGreen[] = {
        {csSetup, 360, clGreen},
        {csEnd}
};

const LedRGBChunk_t lsqAppearRed[] = {
        {csSetup, 360, clRed},
        {csEnd}
};

const LedRGBChunk_t lsqAppearBlue[] = {
        {csSetup, 360, clBlue},
        {csEnd}
};

const LedRGBChunk_t lsqAppearYellow[] = {
        {csSetup, 360, clYellow},
        {csEnd}
};

const LedRGBChunk_t lsqAppearMagenta[] = {
        {csSetup, 360, clMagenta},
        {csEnd}
};

const LedRGBChunk_t lsqAppearCyan[] = {
        {csSetup, 360, clCyan},
        {csEnd}
};

const LedRGBChunk_t lsqAppearWhite[] = {
        {csSetup, 360, clWhite},
        {csEnd}
};
#endif
