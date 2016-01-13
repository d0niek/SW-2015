//
// Created by d0niek on 11/25/15.
//

#include "counter.h"
#include "pre_emptive_os/api/general.h"
#include "printf_P.h"
#include "startup/config.h"
#include "util/functions.h"
#include "util/adc/adc.h"
#include <lpc2xxx.h>

#define KEY_A 0x00100000
#define KEY_B 0x00400000

tS32 _enter = 0;
tS32 _exit = 0;
tS32 almostEnter = 0;
tS32 almostExit = 0;

tS16 refXvalue;
tS16 refYvalue;
tS16 refZvalue;

void checkCrossA(tS32 *crossA);
void checkCrossB(tS32 *crossB);
_Bool isEarthquake();

/**
 *
 */
void initAcc()
{
	refXvalue = getAnalogueInput1(ACCEL_X);
	refYvalue = getAnalogueInput1(ACCEL_Y);
	refZvalue = getAnalogueInput0(ACCEL_Z);
}

/*****************************************************************************
 *
 * Description:
 *    ...
 *
 * Params:
 *    [in] enters
 *    [in] exits
 *
 ****************************************************************************/
void counter(struct Value *enters, struct Value *exits)
{
    tS32 crossA = 0;
    tS32 crossB = 0;

    if (enters->current > 999) {
        printf("Exceeded the maximum value of enters\n");
        enters->current = 0;
        setLast(enters);
    }

    if (exits->current > 999) {
        printf("Exceeded the maximum value of exits\n");
        exits->current = 0;
        setLast(exits);
    }

    checkCrossA(&crossA);
    checkCrossB(&crossB);

    if (!_enter && !_exit) {
        if (crossA && !crossB) {
            printf("Coming in\n");
            _enter = 1;
        } else if (!crossA && crossB) {
            printf("Coming out\n");
            _exit = 1;
        }
    }

    if (_enter && !crossA && crossB) {
        printf("Almost enter\n");
        almostEnter = 1;
    }

    if (_exit && crossA && !crossB) {
        printf("Almost exit\n");
        almostExit = 1;
    }

    if (!crossA && !crossB) {
        _enter = 0;
        _exit = 0;

        IOSET = 0x00260000;

        if (almostEnter) {
            printf("Entered\n");
            almostEnter = 0;
            enters->current += 1;
            IOCLR = 0x00040000; // Blue led
            udelay(300);
        }

        if (almostExit) {
            printf("Went\n");
            almostExit = 0;
            exits->current += 1;
            IOCLR = 0x00020000; // Red led
            udelay(300);
        }
    }
}

void checkCrossA(tS32 *crossA)
{
	// Detect if P1.20 key is pressed
	if ((IOPIN1 & KEY_A) == 0 && !isEarthquake()) {
		IOCLR1 = 0x00010000;
		*crossA = 1;
	} else {
		IOSET1 = 0x00010000;
	}

//	if ((IOPIN & GATE_A) != 0 && !isEarthquake()) {
//		IOSET1 = 0x00010000;
//		*crossA = 1;
//	} else {
//		IOCLR1 = 0x00010000;
//	}
}

void checkCrossB(tS32 *crossB)
{
	// Detect if P1.22 key is pressed
	if ((IOPIN1 & KEY_B) == 0 && !isEarthquake()) {
		IOCLR1 = 0x00040000;
		*crossB = 1;
	} else {
		IOSET1 = 0x00040000;
	}

//	if((IOPIN & GATE_B) != 0 && !isEarthquake()) {
//		IOSET1 = 0x00040000;
//		*crossB = 1;
//	} else {
//		IOCLR1 = 0x00040000;
//	}
}

/**
 *
 */
_Bool isEarthquake()
{
	tS16 Xvalue = getAnalogueInput1(ACCEL_X);
	tS16 Yvalue = getAnalogueInput1(ACCEL_Y);
	tS16 Zvalue = getAnalogueInput0(ACCEL_Z);

	tS16 deviation = 20;

	if (Yvalue - refYvalue >= deviation ||
		Xvalue - refXvalue <= -deviation ||
		Yvalue - refYvalue <= -deviation ||
		Xvalue - refXvalue >= deviation) {
		printf("is Earthquake\n");

		return TREU;
	} else {
		printf("is not Earthquake\n");

		return FALSE;
	}
}
