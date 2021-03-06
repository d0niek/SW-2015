/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2007 Embedded Artists AB
 *
 * Description:
 *    Main program for LPC2148 Education Board test program
 *
 *****************************************************************************/

#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>
#include <lpc2xxx.h>
#include <consol.h>
#include "util/i2c.h"
#include "util/adc/adc.h"
#include "util/lcd/lcd.h"
#include "pca9532.h"
#include "ea_97x60c.h"
#include "value.h"
#include "counter.h"

#define PROC1_STACK_SIZE 1024
#define PROC2_STACK_SIZE 1024
#define INIT_STACK_SIZE  400

static tU8 proc1Stack[PROC1_STACK_SIZE];
static tU8 proc2Stack[PROC2_STACK_SIZE];
static tU8 initStack[INIT_STACK_SIZE];
static tU8 pid1;
static tU8 pid2;

struct Value enters = {0, 0};
struct Value exits = {0, 0};

static void initProc(void *arg);
static void proc1(void *arg);
static void proc2(void *arg);
static void displayResult(tS32 result, tS32 position);

volatile tU32 msClock;
volatile tU8 killProc1 = FALSE;
volatile tU8 rgbSpeed = 10;

/*****************************************************************************
 *
 * Description:
 *    The first function to execute
 *
 ****************************************************************************/
int main(void)
{
    tU8 error;
    tU8 pid;

    osInit();
    osCreateProcess(initProc, initStack, INIT_STACK_SIZE, &pid, 1, NULL, &error);
    osStartProcess(pid, &error);

    osStart();

    return 0;
}

/*****************************************************************************
 *
 * Description:
 *    The entry function for the initialization process.
 *
 * Params:
 *    [in] arg - This parameter is not used in this application.
 *
 ****************************************************************************/
static void initProc(void *arg)
{
    tU8 error;

    eaInit();   //initialize printf
    i2cInit();  //initialize I2C

    osCreateProcess(proc1, proc1Stack, PROC1_STACK_SIZE, &pid1, 3, NULL, &error);
    osStartProcess(pid1, &error);

    osCreateProcess(proc2, proc2Stack, PROC2_STACK_SIZE, &pid2, 3, NULL, &error);
    osStartProcess(pid2, &error);

    osDeleteProcess();
}

/*****************************************************************************
 *
 * Description:
 *    A process entry function
 *
 * Params:
 *    [in] arg - This parameter is not used in this application.
 *
 ****************************************************************************/
static void proc1(void *arg)
{
    printf("\n\n");
    printf("*******************************************************\n");
    printf("*                                                     *\n");
    printf("* Traffic Counter                                     *\n");
    printf("* LPC2138 Education Board v1.1 (2009-05-06).          *\n");
    printf("*                                                     *\n");
    printf("* (C) G&C 2015                                        *\n");
    printf("*                                                     *\n");
    printf("*******************************************************\n");

    IODIR |= (1 << 15);  //P0.15

    IODIR |= 0x00260000;  //RGB
    IOSET = 0x00260000;

    IODIR1 |= 0x000F0000;  //LEDs
    IOSET1 = 0x000F0000;
    osSleep(25);
    IOCLR1 = 0x00030000;
    osSleep(20);
    IOCLR1 = 0x00050000;
    osSleep(15);
    IOCLR1 = 0x000c0000;
    osSleep(10);
    IOCLR1 = 0x00090000;
    osSleep(5);
    IOSET1 = 0x000F0000;
    IODIR1 &= ~0x00F00000;  //Keys

    IODIR0 &= ~GATE_A;
    IODIR0 &= ~GATE_B;

    IODIR1 &= ~RESET_BUTTON;

    initAcc();

    for (; ;) {
        tU8 rxChar;

        counter(&enters, &exits);

        if ((IOPIN1 & RESET_BUTTON) == 0) {
            consolSendString("Counter reset \n");
            enters.current = 0;
            exits.current = 0;
        }

        //echo terminal
        if (TRUE == consolGetChar(&rxChar)) {
            consolSendCh(rxChar);
        }

        osSleep(5);

        if (TRUE == killProc1) {
            printf("\nProc #1 kill itself!!!\n");
            osDeleteProcess();
        }
    }
}

/*****************************************************************************
 *
 * Description:
 *    A process entry function
 *
 * Params:
 *    [in] arg - This parameter is not used in this application.
 *
 ****************************************************************************/
static void proc2(void *arg)
{
    tU8 pca9532Present = FALSE;

    osSleep(50);

    //check if connection with PCA9532
    pca9532Present = pca9532Init();

    if (TRUE == pca9532Present) {
        lcdInit();
        lcdColor(0xff, 0x00);
        lcdClrscr();
        lcdIcon(16, 0, 97, 60, _ea_97x60c[2], _ea_97x60c[3], &_ea_97x60c[4]);

        lcdGotoxy(16, 66);
        lcdPuts("Designed and");

        lcdGotoxy(20, 80);
        lcdPuts("produced by");

        lcdGotoxy(50, 96);
        lcdPuts("G&C");

        lcdGotoxy(8, 112);
        lcdPuts("(C)2015 (v1.1)");
    }

    //Initialize ADC
    initAdc();

    T1TCR = 0; // counter disable
    T1PR = 0; // set prescaler /1
    T1MCR = 0; // disable match act
    T1EMR = 0; // disable external match act
    IOSET1 = ((1UL << 25) | (1UL << 24));
    IODIR1 |= ((1UL << 25) | (1UL << 24));

    for (; ;) {
        osSleep(10);
        if (TRUE == pca9532Present) {
            if (enters.current != enters.last) {
                displayResult(enters.current, 0);
                setLast(&enters);
            }

            if (exits.current != exits.last) {
                displayResult(exits.current, 1);
                setLast(&exits);
            }
        } else {
            rgbSpeed = (getAnalogueInput0(AIN1) >> 7) + 3;
        }
    }
}

/*****************************************************************************
 *
 * Description:
 *    The timer tick entry function that is called once every timer tick
 *    interrupt in the RTOS. Observe that any processing in this
 *    function must be kept as short as possible since this function
 *    execute in interrupt context.
 *
 * Params:
 *    [in] elapsedTime - The number of elapsed milliseconds since last call.
 *
 ****************************************************************************/
void appTick(tU32 elapsedTime)
{
    msClock += elapsedTime;
}

/*****************************************************************************
 *
 * Description:
 *    Display on a LCD result in chosen position
 *
 * Params:
 *    [in] result
 *    [in] position
 *
 ****************************************************************************/
static void displayResult(tS32 result, tS32 position)
{
    tU8 liczChar[3];
    liczChar[0] = result / 100 + '0';
    liczChar[1] = (result - ((result / 100) * 100)) / 10 + '0';
    liczChar[2] = result % 10 + '0';

    consolSendString("Counter \n");
    consolSendString(liczChar);
    consolSendString("\n");

    lcdGotoxy(80, position * 20 + 10);
    lcdColor(0xA1, 0x00);
    lcdPuts(liczChar);
}
