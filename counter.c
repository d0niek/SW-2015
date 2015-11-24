//
// Created by d0niek on 11/25/15.
//

#include "counter.h"
#include "pre_emptive_os/api/general.h"

#define LASER_A 0x00100000
#define LASER_B 0x00400000

tS32 enter = 0;
tS32 exit = 0;
tS32 almostEnter = 0;
tS32 almostExit = 0;

/******************************************************************************
 * Function name:		udelay
 *
 * Descriptions:
 *
 * parameters:			delay length
 * Returned value:		None
 *
 *****************************************************************************/
void udelay(unsigned int delayInUs)
{
    // setup timer #1 for delay
    T1TCR = 0x02;          //stop and reset timer
    T1PR = 0x00;          //set prescaler to zero

    T1MR0 = (((long) delayInUs - 1) * (long) CORE_FREQ / 1000) / 1000;

    T1IR = 0xff;          //reset all interrrupt flags
    T1MCR = 0x04;          //stop timer on match
    T1TCR = 0x01;          //start timer

    //wait until delay time has elapsed
    while (T1TCR & 0x01);
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
void counter(Value enters, Value exits)
{
    tS32 crossA = 0;
    tS32 crossB = 0;

    static tU8 cnt;
    tU8 rxChar;

    if (enters.current > 999) {
        printf("Exceeded the maximum value of enters\n");
        enters.current = 0;
        enters.setLast();
    }

    if (exits.current > 999) {
        printf("Exceeded the maximum value of exits\n");
        exits.current = 0;
        exits.setLast();
    }

    // Detect if P1.20 key is pressed
    if ((IOPIN1 & LASER_A) == 0) {
        IOCLR1 = 0x00010000;
        crossA = 1;
    } else {
        IOSET1 = 0x00010000;
    }

    // Detect if P1.22 key is pressed
    if ((IOPIN1 & LASER_B) == 0) {
        IOCLR1 = 0x00040000;
        crossB = 1;
    } else {
        IOSET1 = 0x00040000;
    }

    if (!enter && !exit) {
        if (crossA && !crossB) {
            printf("Coming in\n");
            enter = 1;
        } else if (!crossA && crossB) {
            printf("Coming out\n");
            exit = 1;
        }
    }

    if (enter && !crossA && crossB) {
        printf("Almost enter\n");
        almostEnter = 1;
    }

    if (exit && crossA && !crossB) {
        printf("Almost exit\n");
        almostExit = 1;
    }

    if (!crossA && !crossB) {
        enter = 0;
        exit = 0;

        if (almostEnter) {
            printf("Entered\n");
            almostEnter = 0;
            enters.current += 1;
            IOCLR = 0x00040000; // zapali si? niebieska dioda RGB
            udelay(300);
        }

        if (almostExit) {
            printf("Went\n");
            almostExit = 0;
            exits.current += 1;
            IOCLR = 0x00020000; // zapalenie diody rgb - Czerwona
            udelay(300);
        }
    }
}
