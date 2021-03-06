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

#define KEY_A (1 << 20)
#define KEY_B (1 << 22)

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

/*****************************************************************************
 *
 * Description:
 *    Initialize starting values for accelerometer
 *
 ****************************************************************************/
void initAcc()
{
    refXvalue = getAnalogueInput1(ACCEL_X);
    refYvalue = getAnalogueInput1(ACCEL_Y);
    refZvalue = getAnalogueInput0(ACCEL_Z);
}

/*****************************************************************************
 *
 * Description:
 *    Count enters and exit from "room"
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

    if (crossA && crossB) {
        almostEnter = 0;
        almostExit = 0;
    }

    if (!crossA && !crossB) {
        _enter = 0;
        _exit = 0;

        IOSET = 0x00260000;

        if (almostEnter) {
            printf("Entered\n");
            almostEnter = 0;
            enters->current += 1;
            IOCLR = (1 << 18); // Blue led
            udelay(300);
        }

        if (almostExit) {
            printf("Went\n");
            almostExit = 0;
            exits->current += 1;
            IOCLR = (1 << 17); // Red led
            udelay(300);
        }
    }
}

/**
 *
 * Description:
 *    Check if laser A was cross
 *
 */
void checkCrossA(tS32 *crossA)
{
    if ((IOPIN & GATE_A) != 0 && !isEarthquake()) {
        IOSET1 = (1 << 16);
        *crossA = 1;
    } else {
        IOCLR1 = (1 << 16);
    }
}

/**
 *
 * Description:
 *    Check if laser B was cross
 *
 */
void checkCrossB(tS32 *crossB)
{
    if((IOPIN & GATE_B) != 0 && !isEarthquake()) {
        IOSET1 = (1 << 17);
        *crossB = 1;
    } else {
        IOCLR1 = (1 << 17);
    }
}

/**
 *
 * Description:
 *    Check if lasers and lights barriers are shaking
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

        return TRUE;
    } else {
        printf("is not Earthquake\n");

        return FALSE;
    }
}
