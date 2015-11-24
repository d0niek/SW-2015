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
#include "i2c.h"
#include "adc.h"
#include "lcd.h"
#include "pca9532.h"
#include "ea_97x60c.h"

#define LASER_A 0x00100000
#define LASER_B 0x00400000

#define PROC1_STACK_SIZE 1024
#define PROC2_STACK_SIZE 1024
#define INIT_STACK_SIZE  400

static tU8 proc1Stack[PROC1_STACK_SIZE];
static tU8 proc2Stack[PROC2_STACK_SIZE];
static tU8 initStack[INIT_STACK_SIZE];
static tU8 pid1;
static tU8 pid2;

tS32 wejscia = 0, wyjscia = 0;
tS32 ostatnieWejscia = 0, ostatnieWyjscia = 0;

static void initProc(void *arg);
static void proc1(void *arg);
static void proc2(void *arg);
void rgbLight();
static void displayResult(tS32 result, tS32 position);

volatile tU32 msClock;
volatile tU8 killProc1 = FALSE;
volatile tU8 rgbSpeed = 10;

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
    /*
     * setup timer #1 for delay
     */
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

    IODIR |= 0x00008000;  //P0.15

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

    tS32 wchodze = 0;
    tS32 wychodze = 0;
    tS32 prawieWszedl = 0;
    tS32 prawieWyszedlem = 0;

    for (; ;) {
        tS32 przeciecieA = 0;
        tS32 przeciecieB = 0;

        static tU8 cnt;
        tU8 rxChar;

        if (wejscia > 999) {
            printf("Przekroczono max warto�� wej��. Liczenie od 0\n");
            wejscia = 0;
        }

        if (wyjscia > 999) {
            printf("Przekroczono max warto�� wyj��. Liczenie od 0\n");
            wyjscia = 0;
        }

        //detect if P1.20 key is pressed
        if ((IOPIN1 & LASER_A) == 0) {
            IOCLR1 = 0x00010000;
            przeciecieA = 1;
        } else {
            IOSET1 = 0x00010000;
        }

        //detect if P1.22 key is pressed
        if ((IOPIN1 & LASER_B) == 0) {
            IOCLR1 = 0x00040000;
            przeciecieB = 1;
        } else {
            IOSET1 = 0x00040000;
        }

        if (!wchodze && !wychodze) {
            //sprawdzenie, kt�re by�o pocz�tkowe przeci�cie (tutaj bramkaA, przeciecieA=1, przeciecieB=0)
            if (przeciecieA && !przeciecieB) {
                printf("Wchodze\n");
                wchodze = 1;
            }
                //sprawdzenie, kt�re by�o pocz�tkowe przeci�cie (tutaj bramkaB, przeciecieA=0, przeciecieB=1)
            else if (!przeciecieA && przeciecieB) {
                printf("Wychodze\n");
                wychodze = 1;
            }
        }

        // Sprawdzenie, czy obiekt przecina ostatni� lini� (tutaj bramk�B)
        if (wchodze && !przeciecieA && przeciecieB) {
            printf("Prawie wszedlem\n");
            prawieWszedl = 1;
        }

        //sprawdzenie, czy obiekt przecina ostatni� lini� (tutaj bramk�A)
        if (wychodze && przeciecieA && !przeciecieB) {
            printf("Prawie wyszed�em\n");
            prawieWyszedlem = 1;
        }

        //sprawdzenie, czy nie ma przeci��
        if (!przeciecieA && !przeciecieB) {
            wchodze = 0; //zerowanie pomocnik�w
            wychodze = 0; //zerowanie pomocnik�w

            // je�li by�o prawieWszed�=1, tutaj jest koniec wej�cia
            if (prawieWszedl) {
                printf("Wszed�em ++ \n");
                prawieWszedl = 0; // zerowanie pomocnik�w
                wejscia += 1;
                IOCLR = 0x00040000; // zapali si� niebieska dioda RGB
                udelay(300);
            }

            // je�li by�o prawieWyszed�=1, tutaj jest koniec wyj�cia
            if (prawieWyszedlem) {
                printf("Wyszed�em -- \n");
                prawieWyszedlem = 0; // zerowanie pomocnik�w
                wyjscia += 1;
                IOCLR = 0x00020000; // zapalenie diody rgb - Czerwona
                udelay(300);
            }
        }

        // rgbLight();

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

        lcdGotoxy(0, 96);
        lcdPuts("G&C");

        lcdGotoxy(8, 112);
        lcdPuts("(C)2015 (v1.1)");
    }

    //Initialize ADC
    initAdc();

    T1TCR = 0;            // counter disable
    T1PR = 0;            // set prescaler /1
    T1MCR = 0;            // disable match act
    T1EMR = 0;            // disable external match act
    IOSET1 = ((1UL << 25) | (1UL << 24));
    IODIR1 |= ((1UL << 25) | (1UL << 24));

    for (; ;) {
        osSleep(10);
        if (TRUE == pca9532Present) {
            if (wejscia != ostatnieWejscia) {
                displayResult(wejscia, 1);
                ostatnieWejscia = wejscia;
            }

            if (wyjscia != ostatnieWyjscia) {
                displayResult(wyjscia, 2);
                ostatnieWyjscia = wyjscia;
            }
        } else {
            rgbSpeed = (getAnalogueInput(AIN1) >> 7) + 3;
        }
    }
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

void rgbLight()
{
    static tU8 cnt;

    cnt++;
    if ((cnt % rgbSpeed) == 0) {
        IOSET = 0x00260000;
        if (cnt == rgbSpeed) {
            IOCLR = 0x00020000;
        } else if (cnt == (2 * rgbSpeed)) {
            IOCLR = 0x00040000;
        } else {
            IOCLR = 0x00200000;
            cnt = 0;
        }
    }
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
    liczChar[0] = wynik / 100 + '0';
    liczChar[1] = (wynik - ((wynik / 100) * 100)) / 10 + '0';
    liczChar[2] = wynik % 10 + '0';

    consolSendString("Licznik \n");
    consolSendString(liczChar);
    consolSendString("\n");

    lcdGotoxy(20, pozycja * 40);
    lcdColor(0xA1, 0x00);
    lcdPuts(liczChar);
}
