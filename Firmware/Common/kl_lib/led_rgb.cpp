/*
 * led_rgb.cpp
 *
 *  Created on: 31 рту. 2014 у.
 *      Author: Kreyl
 */

#include "led_rgb.h"
#include "cmd_uart.h"

LedRGB_t Led;

// Timer callback
static void LedTmrCallback(void *p) {
    chSysLockFromIsr();
    Led.IStartSequenceI((const LedChunk_t*)p);
    chSysUnlockFromIsr();
}

void LedRGB_t::Init() {
    R.Init();
    G.Init();
    B.Init();
    // Initial value
    SetColor(clBlack);
}

void LedRGB_t::IStartSequenceI(const LedChunk_t *PLedChunk) {
    static uint32_t t=0;
    // Reset timer
    if(chVTIsArmedI(&ITmr)) chVTResetI(&ITmr);
    // Process the sequence
    while(PLedChunk != nullptr) {
//        Uart.Printf("\rCh %u", PLedChunk->ChunkSort);
        switch(PLedChunk->ChunkSort) {
            case csSetColor:
                if(ICurrColor != PLedChunk->Color) {
                    if(PLedChunk->Time_ms == 0) {   // If smooth time is zero,
                        SetColor(PLedChunk->Color); // set color now,
                        PLedChunk++;                // and goto next chunk
                    }
                    else {
                        // Adjust color
                        ICurrColor.Adjust(&PLedChunk->Color);
                        ISetCurrent();
                        // Check if completed now
                        if(ICurrColor == PLedChunk->Color) {
                            SmoothVar = -1; // Reset SmoothVar before exit
                            PLedChunk++;
                            uint32_t d = chTimeNow() - t;
                            Uart.Printf("\rt=%u", d);
                        }
                        else { // Not completed
                            // Recalculate SmoothVar if needed
                            if(SmoothVar == -1) {
                                t=chTimeNow();
                                ICalcSmoothVar(PLedChunk->Time_ms, (Color_t*)&PLedChunk->Color);
                            }
                            // Calculate time to next adjustment
                            uint32_t DelayR = (ICurrColor.Red   == PLedChunk->Color.Red  )? 0 : ICalcDelay(ICurrColor.Red);
                            uint32_t DelayG = (ICurrColor.Green == PLedChunk->Color.Green)? 0 : ICalcDelay(ICurrColor.Green);
                            uint32_t DelayB = (ICurrColor.Blue  == PLedChunk->Color.Blue )? 0 : ICalcDelay(ICurrColor.Blue);
                            uint32_t Delay = DelayR;
                            if(DelayG > Delay) Delay = DelayG;
                            if(DelayB > Delay) Delay = DelayB;
//                            uint32_t Delay = ICalcDelay(*PMostDifferentChannel);
                            Uart.Printf(" %u", Delay);
                            chVTSetI(&ITmr, MS2ST(Delay), LedTmrCallback, (void*)PLedChunk);
                            return;
                        } // Not completed
                    } // if time > 256
                } // if color is different
                else PLedChunk++; // Color is the same, goto next chunk
                break;

            case csWait: // Start timer, pointing to next chunk
                chVTSetI(&ITmr, MS2ST(PLedChunk->Time_ms), LedTmrCallback, (void*)(PLedChunk+1));
                return;
                break;

            case csJump:
                PLedChunk = IPStartChunk + PLedChunk->ChunkToJumpTo;
                break;

            case csEnd: return; break;
        } // switch
    } // while
}

void LedRGB_t::ICalcSmoothVar(int32_t DesiredSetupTime, Color_t *PDesiredColor) {
    Uart.Printf("\rClr: %u %u %u", ICurrColor.Red, ICurrColor.Green, ICurrColor.Blue);
    int32_t StartBrt, StopBrt;
    // Which color is most different?
    uint8_t DifRed   = (ICurrColor.Red   > PDesiredColor->Red  )? ICurrColor.Red   - PDesiredColor->Red   : PDesiredColor->Red   - ICurrColor.Red;
    uint8_t DifGreen = (ICurrColor.Green > PDesiredColor->Green)? ICurrColor.Green - PDesiredColor->Green : PDesiredColor->Green - ICurrColor.Green;
    uint8_t DifBlue  = (ICurrColor.Blue  > PDesiredColor->Blue )? ICurrColor.Blue  - PDesiredColor->Blue  : PDesiredColor->Blue  - ICurrColor.Blue;
    Uart.Printf("\rDR=%u; DG=%u; DB=%u", DifRed, DifGreen, DifBlue);
    if(DifRed >= DifGreen and DifRed >= DifBlue) {
        PMostDifferentChannel = &ICurrColor.Red;
        StartBrt = ICurrColor.Red;
        StopBrt  = PDesiredColor->Red;
    }
    else if(DifGreen >= DifRed and DifGreen >= DifBlue) {
        PMostDifferentChannel = &ICurrColor.Green;
        StartBrt = ICurrColor.Green;
        StopBrt  = PDesiredColor->Green;
    }
    else {
        PMostDifferentChannel = &ICurrColor.Blue;
        StartBrt = ICurrColor.Blue;
        StopBrt  = PDesiredColor->Blue;
    }
    int32_t Diff = ABS(StartBrt - StopBrt);
    if(DesiredSetupTime < Diff) SmoothVar = 0;
    else {
        int32_t SumDif = LedHarmonicNumber[StartBrt] - LedHarmonicNumber[StopBrt];
        if(SumDif < 0) SumDif = -SumDif;
        SmoothVar = ((DesiredSetupTime - Diff) * 1000) / SumDif;
        Uart.Printf("\rS=%d; P=%d; Dif=%d; SumDif=%d; t=%u; SmoothVar=%d", StartBrt, StopBrt, Diff, SumDif, DesiredSetupTime, SmoothVar);
    }
}


#if 1 // ============================= LED Channel =============================
void LedChnl_t::Init() const {
    // ==== GPIO setup ====
    if(PTimer == TIM2) PinSetupAlterFunc(PGpio, Pin, omPushPull, pudNone, AF1);
    else if(PTimer == TIM3 or PTimer == TIM4) PinSetupAlterFunc(PGpio, Pin, omPushPull, pudNone, AF2);
    else PinSetupAlterFunc(PGpio, Pin, omPushPull, pudNone, AF3);

    // ==== Timer setup ====
    if     (PTimer == TIM2)  { rccEnableTIM2(FALSE); }
    else if(PTimer == TIM3)  { rccEnableTIM3(FALSE); }
    else if(PTimer == TIM4)  { rccEnableTIM4(FALSE); }
    else if(PTimer == TIM9)  { rccEnableTIM9(FALSE); }
    else if(PTimer == TIM10) { rccEnableAPB2(RCC_APB2ENR_TIM10EN, FALSE); }
    else if(PTimer == TIM11) { rccEnableAPB2(RCC_APB2ENR_TIM11EN, FALSE); }

    PTimer->CR1 = TIM_CR1_CEN; // Enable timer, set clk division to 0, AutoReload not buffered
    PTimer->CR2 = 0;
    PTimer->ARR = LED_TOP_VALUE;

    // ==== Timer's channel ====
#if LED_INVERTED_PWM
#define PwmMode 0b111
#else
#define PwmMode 0b110
#endif
    switch(TmrChnl) {
        case 1:
            PTimer->CCMR1 |= (PwmMode << 4);
            PTimer->CCER  |= TIM_CCER_CC1E;
            break;
        case 2:
            PTimer->CCMR1 |= (PwmMode << 12);
            PTimer->CCER  |= TIM_CCER_CC2E;
            break;
        case 3:
            PTimer->CCMR2 |= (PwmMode << 4);
            PTimer->CCER  |= TIM_CCER_CC3E;
            break;
        case 4:
            PTimer->CCMR2 |= (PwmMode << 12);
            PTimer->CCER  |= TIM_CCER_CC4E;
            break;
        default: break;
    }
}
#endif

#if 1 // ========================= Harmonic numbers ============================
const int16_t LedHarmonicNumber[256] = {
        250, // 0
        450, // 1
        616, // 2
        759, // 3
        884, // 4
        995, // 5
        1095, // 6
        1186, // 7
        1269, // 8
        1346, // 9
        1418, // 10
        1484, // 11
        1547, // 12
        1606, // 13
        1661, // 14
        1714, // 15
        1764, // 16
        1812, // 17
        1857, // 18
        1900, // 19
        1942, // 20
        1982, // 21
        2021, // 22
        2058, // 23
        2093, // 24
        2128, // 25
        2161, // 26
        2193, // 27
        2225, // 28
        2255, // 29
        2284, // 30
        2313, // 31
        2341, // 32
        2368, // 33
        2394, // 34
        2420, // 35
        2445, // 36
        2469, // 37
        2493, // 38
        2516, // 39
        2539, // 40
        2561, // 41
        2583, // 42
        2604, // 43
        2625, // 44
        2645, // 45
        2665, // 46
        2685, // 47
        2704, // 48
        2723, // 49
        2742, // 50
        2760, // 51
        2778, // 52
        2795, // 53
        2812, // 54
        2829, // 55
        2846, // 56
        2862, // 57
        2879, // 58
        2894, // 59
        2910, // 60
        2925, // 61
        2941, // 62
        2956, // 63
        2970, // 64
        2985, // 65
        2999, // 66
        3013, // 67
        3027, // 68
        3041, // 69
        3054, // 70
        3068, // 71
        3081, // 72
        3094, // 73
        3106, // 74
        3119, // 75
        3132, // 76
        3144, // 77
        3156, // 78
        3168, // 79
        3180, // 80
        3192, // 81
        3204, // 82
        3215, // 83
        3226, // 84
        3238, // 85
        3249, // 86
        3260, // 87
        3271, // 88
        3281, // 89
        3292, // 90
        3303, // 91
        3313, // 92
        3323, // 93
        3333, // 94
        3344, // 95
        3354, // 96
        3363, // 97
        3373, // 98
        3383, // 99
        3393, // 100
        3402, // 101
        3412, // 102
        3421, // 103
        3430, // 104
        3439, // 105
        3448, // 106
        3457, // 107
        3466, // 108
        3475, // 109
        3484, // 110
        3493, // 111
        3501, // 112
        3510, // 113
        3518, // 114
        3527, // 115
        3535, // 116
        3543, // 117
        3551, // 118
        3560, // 119
        3568, // 120
        3576, // 121
        3584, // 122
        3592, // 123
        3599, // 124
        3607, // 125
        3615, // 126
        3622, // 127
        3630, // 128
        3637, // 129
        3645, // 130
        3652, // 131
        3660, // 132
        3667, // 133
        3674, // 134
        3681, // 135
        3689, // 136
        3696, // 137
        3703, // 138
        3710, // 139
        3717, // 140
        3724, // 141
        3730, // 142
        3737, // 143
        3744, // 144
        3751, // 145
        3757, // 146
        3764, // 147
        3771, // 148
        3777, // 149
        3784, // 150
        3790, // 151
        3796, // 152
        3803, // 153
        3809, // 154
        3815, // 155
        3822, // 156
        3828, // 157
        3834, // 158
        3840, // 159
        3846, // 160
        3852, // 161
        3858, // 162
        3864, // 163
        3870, // 164
        3876, // 165
        3882, // 166
        3888, // 167
        3894, // 168
        3900, // 169
        3905, // 170
        3911, // 171
        3917, // 172
        3922, // 173
        3928, // 174
        3934, // 175
        3939, // 176
        3945, // 177
        3950, // 178
        3956, // 179
        3961, // 180
        3966, // 181
        3972, // 182
        3977, // 183
        3982, // 184
        3988, // 185
        3993, // 186
        3998, // 187
        4003, // 188
        4009, // 189
        4014, // 190
        4019, // 191
        4024, // 192
        4029, // 193
        4034, // 194
        4039, // 195
        4044, // 196
        4049, // 197
        4054, // 198
        4059, // 199
        4064, // 200
        4069, // 201
        4074, // 202
        4079, // 203
        4083, // 204
        4088, // 205
        4093, // 206
        4098, // 207
        4102, // 208
        4107, // 209
        4112, // 210
        4116, // 211
        4121, // 212
        4126, // 213
        4130, // 214
        4135, // 215
        4139, // 216
        4144, // 217
        4148, // 218
        4153, // 219
        4157, // 220
        4162, // 221
        4166, // 222
        4171, // 223
        4175, // 224
        4179, // 225
        4184, // 226
        4188, // 227
        4192, // 228
        4197, // 229
        4201, // 230
        4205, // 231
        4209, // 232
        4214, // 233
        4218, // 234
        4222, // 235
        4226, // 236
        4230, // 237
        4234, // 238
        4238, // 239
        4243, // 240
        4247, // 241
        4251, // 242
        4255, // 243
        4259, // 244
        4263, // 245
        4267, // 246
        4271, // 247
        4275, // 248
        4279, // 249
        4283, // 250
        4287, // 251
        4291, // 252
        4294, // 253
        4298, // 254
        4302, // 255
};
#endif
