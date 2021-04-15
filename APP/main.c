/*
 * main.c
 *
 *  Created on: Jan 29, 2021
 *      Author: Kishk
 *
 */

/* INCLUDING DRIVERS */
#include "../Services/STD_TYPES.h"
#include "../Services/BIT_MATHS.h"
#include "util/delay.h"
#include "../MCAL/01-DIO/DIO_interface.h"									//DIO DRIVER
#include "../MCAL/02- EXTI/EXTI_interface.h"								//EXTERNAL INTERRUPT DRIVER
#include "../MCAL/03- GIE/GIE_interface.h"


/* DEFINE TIMER0 REGISTERS AND BITS */
#define TCCR0   (* (volatile u8 *) (0x53))
#define TCNT0   (* (volatile u8 *) (0x52))
#define OCR0   (* (volatile u8 *) (0x5C))
#define TIMSK   (* (volatile u8 *) (0x59))
#define TIFR   (* (volatile u8 *) (0x58))

#define WGM01	3
#define CS00	0
#define CS01	1
#define CS02	2

#define OCIE0	1

/* DEFINE FUNCTION PROTOTYPE */
void Timer_Init(void);														//TIMER0 INITIALIZATION PROTOTYPE
void Button_ISR(void);														//BUTTON ISR FUNCTION

volatile u8 u8Color = 0;													//VARIABLE TO SWITCH COLORS
volatile u8 u8ButtonFlag = 0;												//BUTTON PRESSED FLAG
volatile u8 u8BlinkFlag = 0;												//BLINK ON FLAG
volatile u8 counter = 0;													//COUNTER FOR TIMER0

void __vector_10(void) __attribute__((signal));								//TIMER0 INTERRUPT VECTOR FUNCTION (OUTPUT COMPARE MATCH MODE)

int main(void) {

	DIO_enumSetPinDirection(DIO_PORTB_PIN0, DIO_OUTPUT);					//INITIALIZE RED LED AS OUTPUT
	DIO_enumSetPinDirection(DIO_PORTB_PIN1, DIO_OUTPUT);					//INITIALIZE GREEB LED AS OUTPUT
	DIO_enumSetPinDirection(DIO_PORTB_PIN2, DIO_OUTPUT);					//INITIALIZE BLUE LED AS OUTPUT
	DIO_enumSetPinDirection(DIO_PORTD_PIN2, DIO_INPUT);						//INITIALIZE BUTTON AS INPPUT
	DIO_voidActivePullUp(DIO_PORTD_PIN2);									//ACTIVE INTERNAL PULLUP RESISTOR FOR BUTTON
	EXTI_voidCallBack(Button_ISR, EXTI_LINE0);								//INIALIZE ISR FUNCTION (FALLING EDGE)
	EXTI_voidInit();														//INITALIZE EXTERNAL INTERRUPT
	Timer_Init();															//INITIALIZE TIMER0
	GIE_void_Enable();														//ENABLE GLOBAL INTERRUPT
	while (1) {
		/* IF BLINKING IS ENABLED */
		if (u8BlinkFlag) {
			DIO_enumSetPinValue(DIO_PORTB_PIN0, DIO_LOW);
			DIO_enumSetPinValue(DIO_PORTB_PIN1, DIO_LOW);
			DIO_enumSetPinValue(DIO_PORTB_PIN2, DIO_LOW);
			switch (u8Color) {
			case 0:
				DIO_enumSetPinValue(DIO_PORTB_PIN0, DIO_HIGH);
				DIO_enumSetPinValue(DIO_PORTB_PIN1, DIO_HIGH);
				DIO_enumSetPinValue(DIO_PORTB_PIN2, DIO_HIGH);
				_delay_ms(100);
				if (!u8BlinkFlag){
					break;
				}
				DIO_enumSetPinValue(DIO_PORTB_PIN0, DIO_LOW);
				DIO_enumSetPinValue(DIO_PORTB_PIN1, DIO_LOW);
				DIO_enumSetPinValue(DIO_PORTB_PIN2, DIO_LOW);
				_delay_ms(100);
				break;

			case 1:

				DIO_enumSetPinValue(DIO_PORTB_PIN0, DIO_HIGH);
				_delay_ms(100);
				if (!u8BlinkFlag){
					break;
				}
				DIO_enumSetPinValue(DIO_PORTB_PIN0, DIO_LOW);
				_delay_ms(100);
				break;

			case 2:
				DIO_enumSetPinValue(DIO_PORTB_PIN1, DIO_HIGH);
				_delay_ms(100);
				if (!u8BlinkFlag){
					break;
				}
				DIO_enumSetPinValue(DIO_PORTB_PIN1, DIO_LOW);
				_delay_ms(100);
				break;

			case 3:
				DIO_enumSetPinValue(DIO_PORTB_PIN2, DIO_HIGH);
				_delay_ms(100);
				if (!u8BlinkFlag){
					break;
				}
				DIO_enumSetPinValue(DIO_PORTB_PIN2, DIO_LOW);
				_delay_ms(100);

				break;

			}
		}

	}

}

void Timer_Init(void) {
	SET_BIT(TCCR0, WGM01);											//ENABLE OUTPUT COMPARE MATCH TIMER0 MODE
	SET_BIT(TIMSK, OCIE0);											//ENABLE INTERRUPT IN OUTPUT COMPARE MATCH
	OCR0 = 255;														//SET TOP VALUE 255
}

void Button_ISR(void) {
	u8ButtonFlag++;													//SAVE NUMBER OF PRESSING BUTTON

	if (u8ButtonFlag == 2) {										//IF BUTTON PRESSED 2 TIMES CONSECUTIVE (BLINKING ENABLE)
		u8BlinkFlag = 1;											//SET BLINKING FLAG
		CLR_BIT(TCCR0, CS00);										//DISABLE TIMER0
		CLR_BIT(TCCR0, CS01);										//DISABLE TIMER0
	} else {
		OCR0 = 255;													//SET TOP VALUE OF TIMER0 -> 255

		TCCR0 |= (1 << CS00) | (1 << CS01);							//ENABLE TIMER0
	}

}

void __vector_10(void) {
	if (counter < 90) {												//COUNTER COUNT UP TO ~3 SECONDS TO CHECK IF USER HOLD BUTTON
		counter++;													//INCREMENTING THE COUNTER
	}
	/* CHECK IF USER RELEASE HIS HAND FROM THE BUTTON AND JUST PRESSED ONE TIME IN ALMOST 1 SECONDS */
	if (u8ButtonFlag != 2 && counter >= 25 && DIO_u8ReadPinValue(DIO_PORTD_PIN2)) {
		counter = 0;												//RESET COUNTER
		u8ButtonFlag = 0;											//RESET BUTTON FLAG
		u8BlinkFlag = 0;											//RESET BLINKING FLAG
		CLR_BIT(TCCR0, CS00);										//DISABLE TIMER0
		CLR_BIT(TCCR0, CS01);										//DISABLE TIMER0
	/*SWITCH CASE TO SWITCH THE COLOR*/
		switch (u8Color) {
		/*RED CASE*/
		case 0:
			DIO_enumSetPinValue(DIO_PORTB_PIN1, DIO_LOW);
			DIO_enumSetPinValue(DIO_PORTB_PIN2, DIO_LOW);
			_delay_ms(15);
			DIO_enumSetPinValue(DIO_PORTB_PIN0, DIO_HIGH);

			u8Color++;
			break;
		/*GREEN CASE*/
		case 1:
			DIO_enumSetPinValue(DIO_PORTB_PIN0, DIO_LOW);
			DIO_enumSetPinValue(DIO_PORTB_PIN2, DIO_LOW);
			_delay_ms(15);
			DIO_enumSetPinValue(DIO_PORTB_PIN1, DIO_HIGH);

			u8Color++;
			break;
		/*BLUE CASE*/
		case 2:
			DIO_enumSetPinValue(DIO_PORTB_PIN0, DIO_LOW);
			DIO_enumSetPinValue(DIO_PORTB_PIN1, DIO_LOW);
			_delay_ms(15);
			DIO_enumSetPinValue(DIO_PORTB_PIN2, DIO_HIGH);
			u8Color++;
			break;
		/*WHITE CASE*/
		case 3:
			DIO_enumSetPinValue(DIO_PORTB_PIN0, DIO_HIGH);
			DIO_enumSetPinValue(DIO_PORTB_PIN1, DIO_HIGH);
			DIO_enumSetPinValue(DIO_PORTB_PIN2, DIO_HIGH);

			u8Color = 0;
			break;
		}
	}

	/*CASE  THE USER HOLD THE BUTTON FOR 3 SECONDS*/
	else if(u8ButtonFlag != 2 && counter >= 90 && !DIO_u8ReadPinValue(DIO_PORTD_PIN2)){
		counter = 0;												//RESET COUNTER
		u8ButtonFlag = 0;											//RESET BUTTON FLAG
		u8BlinkFlag = 0;											//RESET BLINK FLAG
		CLR_BIT(TCCR0, CS00);										//DISABLE TIMER0
		CLR_BIT(TCCR0, CS01);										//DISABLE TIMER1
		/* TURN OFF THE LED*/
		DIO_enumSetPinValue(DIO_PORTB_PIN0, DIO_LOW);
		DIO_enumSetPinValue(DIO_PORTB_PIN1, DIO_LOW);
		DIO_enumSetPinValue(DIO_PORTB_PIN2, DIO_LOW);
	}
}
