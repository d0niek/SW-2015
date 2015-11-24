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
#define STEP_DELAY	1	//ustawienie ile ms op�nienia
#define KEY_DELAY	4000	//ustawienie op�nienia dla przycisku (4s)
#define przycisk     1<<20 //przycisk P1.20;

/*****************************************************************************
 *
 * Description:
 *    Op�nienie
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
 *    Zapala/Gasi diody, w zale�no�ci od 'stan', dodatkowo op�nienie
 *
 ****************************************************************************/
static void
zapalanie(tU8 stan)
{
	setPca9532Pin(0, stan); //zapalenie lewej g�rnej diody
	setPca9532Pin(7, stan); //zapalenie lewej dolnej diody
	setPca9532Pin(8, stan); //zapalenie prawej g�rnej diody
	setPca9532Pin(15, stan); //zapalenie prawej dolnej diody
    delayMs(STEP_DELAY); //op�nienie
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
 *    Wprowadza informacje o ilo�ci przej�� na LCD, w zale�no�ci od warto�ci
 *		'pozycja' b�dzie to albo wej�cie, albo wyj�cie
 *
 ****************************************************************************/
static void wyswietlWynik(tS32 wynik, tS32 pozycja) //wy�wietlanie wyniku dla wyj�� i wej��;
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
 *    G��wna funkcja programu
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
			
	/*Ustawienie pin�w fototranzystor�w na wej�cie*/
	IODIR0 &= ~bramkaA;
	IODIR0 &= ~bramkaB;
	
	/*Ustawienie przycisku P1.20 jako wej�cie*/
	IODIR1 &= ~przycisk;
	
	/*RGB*/
	IODIR |= 0x00260000; //ustawienie diod rgb jako wyj�cie
	IOSET  = 0x00260000; //wy��czenie diod rgb

	//Tekst niezmienialny na wy�wietlacz:
	lcdInit(); //inicjalizujemy wy�wietlacz
	lcdColor(0,0); //ustawiamy kolor
	lcdClrscr(); //czy�cimy wy�wietlacz
	lcdGotoxy(2,20); //pozycja X,Y
  	lcdColor(0xA1,0xB0);
	lcdPuts("Ilosc wejsc:");
	lcdGotoxy(2,60);
  	lcdColor(0xA1,0xB0);
	lcdPuts("Ilosc wyjsc:");
	
	wyswietlWynik(wejscia, 1); //wy�wietlenie wyniku wej��, =0
	wyswietlWynik(wyjscia, 2); //wy�wietlenie wyniku wyj��, =0
	
	while (1) 
	{
		tS32 przeciecieA = 0;
		tS32 przeciecieB = 0;
		IOSET = 0x00260000; //stan niski dla RGB, wy��czenie diody
		
		if(wejscia>999)
		{
		consolSendString("Przekroczono max warto�� wej��. Liczenie od 0");
		wejscia = 0;
		}
	
		if(wyjscia>999)
		{
		consolSendString("Przekroczono max warto�� wyj��. Liczenie od 0");
		wyjscia = 0;
		}
		
		if (((IOPIN1 & przycisk) == 0) && (ostatniStanPrzycisku==0)) //sprawdzenie, czy stan przycisku jest wysoki
												//i czy jego ostatni stan by� == 0
		{	
			delayMs(KEY_DELAY); //op�nienie;
			if((IOPIN1 & przycisk) == 0) //ponowne sprawdzenie stanu
			{
				wejscia = 0;
				wyjscia = 0;
				wyswietlWynik(wejscia, 1);
				wyswietlWynik(wyjscia, 2);
				ostatniStanPrzycisku=1;
				consolSendString("Zerowanie licznik�w\n");
				IOCLR = 0x00200000; //zielona dioda rgb
				delayMs(300);
				
			}
		}
		if((IOPIN1 & przycisk) != 0) 
		{
			ostatniStanPrzycisku=0;
		}
		
		/**FOTOTRANZYSTOR**/
		if ((IOPIN & bramkaA) != 0) //sprawdzanie, czy jest przeci�cie linii �wiat�a
		{	
			przeciecieA = 1;
			zapalanie(0); //zapalanie diod
			zapalanie(1); //gaszenie diod
		}
		
		if((IOPIN & bramkaB) != 0) //sprawdzanie, czy jest przeci�cie linii �wiat�a
		{	
			przeciecieB = 1;
			zapalanie(0); //zapalanie diod
			zapalanie(1); //gaszenie diod
		}

		if (!wchodze && !wychodze) 
		{
			if (przeciecieA && !przeciecieB) //sprawdzenie, kt�re by�o pocz�tkowe przeci�cie (tutaj bramkaA, przeciecieA=1, przeciecieB=0)
			{	
				consolSendString("Wchodz� \n"); //wysy�anie tekstu przez UART
				wchodze = 1;
			} 
			else if (!przeciecieA && przeciecieB) //sprawdzenie, kt�re by�o pocz�tkowe przeci�cie (tutaj bramkaB, przeciecieA=0, przeciecieB=1)
			{	
				consolSendString("Wychodz� \n");
				wychodze = 1;
			}
		}

		if (wchodze && !przeciecieA && przeciecieB) //sprawdzenie, czy obiekt przecina ostatni� lini� (tutaj bramk�B)
		{	
			consolSendString("Prawie wszed�em \n");
			prawieWszedl = 1;
		}
		if (wychodze && przeciecieA && !przeciecieB) //sprawdzenie, czy obiekt przecina ostatni� lini� (tutaj bramk�A)
		{	
			consolSendString("Prawie wyszed�em \n");
			prawieWyszedlem = 1;
		}	

		if (!przeciecieA && !przeciecieB) //sprawdzenie, czy nie ma przeci��
		{	
			wchodze = 0; //zerowanie pomocnik�w
			wychodze = 0;	//zerowanie pomocnik�w
			if (prawieWszedl) //je�li by�o prawieWszed�=1, tutaj jest koniec wej�cia
			{	
				consolSendString("Wszed�em ++ \n");
				prawieWszedl = 0; //zerowanie pomocnik�w
				wejscia += 1;
				wyswietlWynik(wejscia, 1); //wy�wietlenie wyniku dla wej��
				IOCLR = 0x00040000; //zapali si� niebieska dioda RGB
				delayMs(300);
			}
			if (prawieWyszedlem) //je�li by�o prawieWyszed�=1, tutaj jest koniec wyj�cia
			{	
				consolSendString("Wyszed�em -- \n");
				prawieWyszedlem = 0; //zerowanie pomocnik�w
				wyjscia += 1;
				wyswietlWynik(wyjscia, 2); //wy�wietlenie wyniku dla wyj��
				IOCLR = 0x00020000; //zapalenie diody rgb - Czerwona	
				delayMs(300);
			}
		}	
	}
}
