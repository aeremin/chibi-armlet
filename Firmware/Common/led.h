/*
 * led.h
 *
 *  Created on: 03.11.2011
 *      Author: g.kruglov
 */

#ifndef LED_H_
#define LED_H_

#include "stm32l1xx.h"
#include "ch.h"
#include "hal.h"
#include "kl_lib_L15x.h"

// Colors
struct Color_t {
    uint8_t Red, Green, Blue;
    bool operator == (const Color_t AColor) { return ((Red == AColor.Red) and (Green == AColor.Green) and (Blue == AColor.Blue)); }
    //bool operator != (const Color_t AColor) { return ((this->Red != AColor.Red) || (this->Green != AColor.Green) || (this->Blue != AColor.Blue)); }
};
#define clBlack     ((Color_t){0,   0,   0})
#define clRed       ((Color_t){255, 0,   0})
#define clGreen     ((Color_t){0,   255, 0})
#define clBlue      ((Color_t){0,   0,   255})
#define clYellow    ((Color_t){255, 255, 0})
#define clViolet    ((Color_t){255, 0, 255})
#define clCyan      ((Color_t){0, 255, 255})
#define clWhite     ((Color_t){255, 255, 255})

/*
 * RGB LED
 * Provides smooth color change.
 */
#define LED_SETUP_DELAY_MS  45
// Timer
#define LED_TIM         TIM3
#define LED_RED_CCR     CCR2
#define LED_GREEN_CCR   CCR4
#define LED_BLUE_CCR    CCR3
#define LED_RCC_EN()    rccEnableTIM3(FALSE)
#define LED_ALTERFUNC   AF2 // TIM3
// GPIO
#define LED_GPIO        GPIOB
#define LED_P1          0   // }
#define LED_P2          1   // }
#define LED_P3          5   // } No need to diff between colors

class LedRGB_t {
private:
	Color_t ICurrentColor, INeededColor;
	Thread *PThread;
	bool IsSleeping;
	void ISetRed  (uint8_t AValue) {LED_TIM->LED_RED_CCR   = AValue;}
	void ISetGreen(uint8_t AValue) {LED_TIM->LED_GREEN_CCR = AValue;}
	void ISetBlue (uint8_t AValue) {LED_TIM->LED_BLUE_CCR  = AValue;}
public:
	void Init();
	void Task();
	void SetColor(Color_t AColor) {
		ISetRed(AColor.Red);
		ISetGreen(AColor.Green);
		ISetBlue(AColor.Blue);
		ICurrentColor = AColor;
	}
	void SetColorSmoothly(Color_t AColor);
	bool IsOff() { return (ICurrentColor == INeededColor) and (ICurrentColor == clBlack); }
//	void Blink(uint32_t ABlinkDelay, Color_t AColor) {
//	        IsInsideBlink = true;
//	        IBlinkDelay = ABlinkDelay;
//	        INeededColor = AColor;
//	        SetColor(AColor);
//	        Delay.Reset(&ITimer);
//	    }
};

extern LedRGB_t Led;


#endif /* LED_H_ */
