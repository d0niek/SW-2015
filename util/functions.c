/*
 * functions.c
 *
 *  Created on: 2016-01-08
 *      Author: embedded
 */

#include "functions.h"
#include "../startup/config.h"
#include <lpc2xxx.h>

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
