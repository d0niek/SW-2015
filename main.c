#include "i2c.h"
#include "pca9532.h"
#include "lcd.h"
#include "adc.h"
#include "general.h"
#include <lpc2xxx.h>
#include <printf_P.h>
#include <ea_init.h>
#include "startup/config.h"

#define bramkaA 	1<<15 	//ustawienie pinu dla pierwszego fototranzystora
#define bramkaB		1<<10 	//ustawienie pinu dla drugiego fototranzystora
#define STEP_DELAY	1	//ustawienie ile ms opóŸnienia
#define KEY_DELAY	4000	//ustawienie opóŸnienia dla przycisku (4s)
#define przycisk     1<<20 //przycisk P1.20;

/*****************************************************************************
 *
 * Description:
 *    OpóŸnienie
 *
 ****************************************************************************/
static void
delayMs(tU16 delayInMs)
{
  /*
   * setup timer #1 for delay
   */
  T1TCR = 0x02;          //stop and reset timer
  T1PR  = 0x00;          //set prescaler to zero
  T1MR0 = delayInMs * (CORE_FREQ / PBSD / 1000);
  T1IR  = 0xff;          //reset all interrrupt flags
  T1MCR = 0x04;          //stop timer on match
  T1TCR = 0x01;          //start timer
  
  while (T1TCR & 0x01)
    ;
}

/*****************************************************************************
 *
 * Description:
 *    Zapala/Gasi diody, w zale¿noœci od 'stan', dodatkowo opóŸnienie
 *
 ****************************************************************************/
static void
zapalanie(tU8 stan)
{
	setPca9532Pin(0, stan); //zapalenie lewej górnej diody
	setPca9532Pin(7, stan); //zapalenie lewej dolnej diody
	setPca9532Pin(8, stan); //zapalenie prawej górnej diody
	setPca9532Pin(15, stan); //zapalenie prawej dolnej diody
    delayMs(STEP_DELAY); //opóŸnienie
	setPca9532Pin(1, stan);
	setPca9532Pin(6, stan);
	setPca9532Pin(9, stan);
	setPca9532Pin(14, stan);
    delayMs(STEP_DELAY);
	setPca9532Pin(2, stan);
	setPca9532Pin(5, stan);
	setPca9532Pin(10, stan);
	setPca9532Pin(13, stan);
	delayMs(STEP_DELAY);
	setPca9532Pin(3, stan);
	setPca9532Pin(4, stan);
	setPca9532Pin(11, stan);
	setPca9532Pin(12, stan);
	delayMs(STEP_DELAY);
}

/*****************************************************************************
 *
 * Description:
 *    Wprowadza informacje o iloœci przejœæ na LCD, w zale¿noœci od wartoœci
 *		'pozycja' bêdzie to albo wejœcie, albo wyjœcie
 *
 ****************************************************************************/
static void wyswietlWynik(tS32 wynik, tS32 pozycja) //wy¶wietlanie wyniku dla wyj¶æ i wej¶æ;
{
	tU8 liczChar[3];
	liczChar[0] = wynik/100 + '0';
	liczChar[1] = (wynik-((wynik/100)*100))/10 + '0';
	liczChar[2] = wynik%10 + '0';
	consolSendString("Licznik \n");
	consolSendString(liczChar); 
	consolSendString("\n");
	lcdGotoxy(20,pozycja*40);
	lcdColor(0xA1,0x00);
	lcdPuts(liczChar);
}

/*****************************************************************************
 *
 * Description:
 *    G³ówna funkcja programu
 *
 ****************************************************************************/
int main(void) 
{
	tS32 wejscia = 0, wyjscia = 0, ostatniStanPrzycisku = 0;	
	tS32 wchodze = 0;
	tS32 wychodze = 0;
	tS32 prawieWszedl = 0;
	tS32 prawieWyszedlem = 0;

	// initializacja UART
	consolInit();

	//diody led, inicjalizacja i2c
	i2cInit();
	pca9532Init();
			
	/*Ustawienie pinów fototranzystorów na wejœcie*/
	IODIR0 &= ~bramkaA;
	IODIR0 &= ~bramkaB;
	
	/*Ustawienie przycisku P1.20 jako wejœcie*/
	IODIR1 &= ~przycisk;
	
	/*RGB*/
	IODIR |= 0x00260000; //ustawienie diod rgb jako wyj¶cie
	IOSET  = 0x00260000; //wy³±czenie diod rgb

	//Tekst niezmienialny na wy¶wietlacz:
	lcdInit(); //inicjalizujemy wy¶wietlacz
	lcdColor(0,0); //ustawiamy kolor
	lcdClrscr(); //czy¶cimy wy¶wietlacz
	lcdGotoxy(2,20); //pozycja X,Y
  	lcdColor(0xA1,0xB0);
	lcdPuts("Ilosc wejsc:");
	lcdGotoxy(2,60);
  	lcdColor(0xA1,0xB0);
	lcdPuts("Ilosc wyjsc:");
	
	wyswietlWynik(wejscia, 1); //wy¶wietlenie wyniku wej¶æ, =0
	wyswietlWynik(wyjscia, 2); //wy¶wietlenie wyniku wyj¶æ, =0
	
	while (1) 
	{
		tS32 przeciecieA = 0;
		tS32 przeciecieB = 0;
		IOSET = 0x00260000; //stan niski dla RGB, wy³±czenie diody
		
		if(wejscia>999)
		{
		consolSendString("Przekroczono max wartoœæ wejœæ. Liczenie od 0");
		wejscia = 0;
		}
	
		if(wyjscia>999)
		{
		consolSendString("Przekroczono max wartoœæ wyjœæ. Liczenie od 0");
		wyjscia = 0;
		}
		
		if (((IOPIN1 & przycisk) == 0) && (ostatniStanPrzycisku==0)) //sprawdzenie, czy stan przycisku jest wysoki
												//i czy jego ostatni stan by³ == 0
		{	
			delayMs(KEY_DELAY); //opóŸnienie;
			if((IOPIN1 & przycisk) == 0) //ponowne sprawdzenie stanu
			{
				wejscia = 0;
				wyjscia = 0;
				wyswietlWynik(wejscia, 1);
				wyswietlWynik(wyjscia, 2);
				ostatniStanPrzycisku=1;
				consolSendString("Zerowanie liczników\n");
				IOCLR = 0x00200000; //zielona dioda rgb
				delayMs(300);
				
			}
		}
		if((IOPIN1 & przycisk) != 0) 
		{
			ostatniStanPrzycisku=0;
		}
		
		/**FOTOTRANZYSTOR**/
		if ((IOPIN & bramkaA) != 0) //sprawdzanie, czy jest przeciêcie linii ¶wiat³a
		{	
			przeciecieA = 1;
			zapalanie(0); //zapalanie diod
			zapalanie(1); //gaszenie diod
		}
		
		if((IOPIN & bramkaB) != 0) //sprawdzanie, czy jest przeciêcie linii ¶wiat³a
		{	
			przeciecieB = 1;
			zapalanie(0); //zapalanie diod
			zapalanie(1); //gaszenie diod
		}

		if (!wchodze && !wychodze) 
		{
			if (przeciecieA && !przeciecieB) //sprawdzenie, które by³o pocz±tkowe przeciêcie (tutaj bramkaA, przeciecieA=1, przeciecieB=0)
			{	
				consolSendString("Wchodzê \n"); //wysy³anie tekstu przez UART
				wchodze = 1;
			} 
			else if (!przeciecieA && przeciecieB) //sprawdzenie, które by³o pocz±tkowe przeciêcie (tutaj bramkaB, przeciecieA=0, przeciecieB=1)
			{	
				consolSendString("Wychodzê \n");
				wychodze = 1;
			}
		}

		if (wchodze && !przeciecieA && przeciecieB) //sprawdzenie, czy obiekt przecina ostatni± liniê (tutaj bramkêB)
		{	
			consolSendString("Prawie wszed³em \n");
			prawieWszedl = 1;
		}
		if (wychodze && przeciecieA && !przeciecieB) //sprawdzenie, czy obiekt przecina ostatni± liniê (tutaj bramkêA)
		{	
			consolSendString("Prawie wyszed³em \n");
			prawieWyszedlem = 1;
		}	

		if (!przeciecieA && !przeciecieB) //sprawdzenie, czy nie ma przeciêæ
		{	
			wchodze = 0; //zerowanie pomocników
			wychodze = 0;	//zerowanie pomocników
			if (prawieWszedl) //je¶li by³o prawieWszed³=1, tutaj jest koniec wej¶cia
			{	
				consolSendString("Wszed³em ++ \n");
				prawieWszedl = 0; //zerowanie pomocników
				wejscia += 1;
				wyswietlWynik(wejscia, 1); //wy¶wietlenie wyniku dla wej¶æ
				IOCLR = 0x00040000; //zapali siê niebieska dioda RGB
				delayMs(300);
			}
			if (prawieWyszedlem) //je¶li by³o prawieWyszed³=1, tutaj jest koniec wyj¶cia
			{	
				consolSendString("Wyszed³em -- \n");
				prawieWyszedlem = 0; //zerowanie pomocników
				wyjscia += 1;
				wyswietlWynik(wyjscia, 2); //wy¶wietlenie wyniku dla wyj¶æ
				IOCLR = 0x00020000; //zapalenie diody rgb - Czerwona	
				delayMs(300);
			}
		}	
	}
}
