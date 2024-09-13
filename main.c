#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
/*************************************************************/
#define SET_BIT(REG , BIT)      ( REG |=  (1 << (BIT)) )
#define CLR_BIT(REG , BIT)      ( REG &= ~(1 << (BIT)) )
#define TOG_BIT(REG , BIT)      ( REG ^=  (1 << (BIT)) )
#define GET_BIT(REG , BIT)      ( (REG >> BIT) & 0X01  )
/*************************************************************/
unsigned char sec_units  = 0 ; /* Legal Values [0:9] */
unsigned char min_units  = 0 ; /* Legal Values [0:9] */
unsigned char hour_units = 0 ; /* Legal Values [0:9] */
unsigned char sec_tens   = 0 ; /* Legal Values [0:5] */
unsigned char min_tens   = 0 ; /* Legal Values [0:5] */
unsigned char hour_tens  = 0 ; /* Legal Values [0:2] */
unsigned char Timer_Falg = 0 ; /* indicated for triggered interrupt */
unsigned char UP_DW_bar  = 1 ; /* To differentiate between the counting modes */

/*************************************************************/
void Timer_1_CTC_Mode_INIT( void )
{
	SREG |= 1<<7 ; /* Global Interrupt Enable */
	TCCR1A = (1<<FOC1A) | (1<<FOC1B) ; /* non PWM mode */
	TCCR1B = (1<<WGM12) | (1<<CS10) | (1<<CS12) ; /* 1024 Prescaler */
	TCNT1  = 0 ; /* start counting from zero */
	OCR1A  = 15625 ; /* Number of counts required for the timer to trigger an interrupt every one second */
	TIMSK |= 1<<OCIE1A ;  /* Module interrupt Enable */

}
/*************************************************************/
ISR(TIMER1_COMPA_vect)
{
	Timer_Falg = 1 ; /* simple ISR to avoid a huge interrupts latency */
}
/*************************************************************/
void INT0_INIT( void )
{
	MCUCR |= 1<<ISC01 ;  /* interrupt to be triggered with falling edge */
	GICR  |= 1<<INT0  ; /* module interrupt Enable */

}

/*************************************************************/
ISR(INT0_vect)
{
	sec_units = 0 ;
	sec_tens  = 0 ;

	min_units = 0 ;
	min_tens  = 0 ;

	hour_units = 0 ;
	hour_tens  = 0 ;
	CLR_BIT(PORTD , PD0) ; /* to clear the buzzer press on reset */
	UP_DW_bar  = 1 ; /* start count up again */

}
/*************************************************************/
void INT1_INIT( void )
{
	MCUCR |= (1<<ISC11) | (1<<ISC10) ; /* interrupt to be triggered with rising edge */
	GICR  |= 1<<INT1  ; /* module interrupt Enable */
}

/*************************************************************/
ISR(INT1_vect)
{
	/* stop the timer using clock bits */
	CLR_BIT(TCCR1B , CS10) ;
	CLR_BIT(TCCR1B , CS11) ;
	CLR_BIT(TCCR1B , CS12) ;
}
/*************************************************************/
void INT2_INIT( void )
{
	CLR_BIT(MCUCSR , ISC2) ; /* interrupt to be triggered with falling edge */
	GICR  |= 1<<INT2  ; /* module interrupt Enable */

}

/*************************************************************/
ISR(INT2_vect)
{
	/* resume the timer by enable the clock bits */
	SET_BIT(TCCR1B , CS10) ;
	SET_BIT(TCCR1B , CS12) ;
}
/*************************************************************/
void update_state( char UP_DW_bar )
{
	if(UP_DW_bar)
	{
		SET_BIT(PORTD , PD4);
		CLR_BIT(PORTD , PD5);

		++sec_units ;
		if(sec_units == 10 )
		{
			sec_units = 0 ;
			++sec_tens ;
			if(sec_tens == 6)
			{
				sec_tens = 0 ;
				++min_units ;
				if(min_units == 10)
				{
					min_units = 0 ;
					++min_tens ;
					if(min_tens == 6)
					{
						min_tens = 0 ;
						++hour_units ;
						if(hour_units == 10)
						{
							hour_units = 0 ;
							++hour_tens ;
							if(hour_tens == 3)
							{
								sec_units = 0 ;
								sec_tens  = 0 ;

								min_units = 0 ;
								min_tens  = 0 ;

								hour_units = 0 ;
								hour_tens  = 0 ;

							}
						}
					}
				}
			}
		}

	}
	else
	{
		SET_BIT(PORTD , PD5);
		CLR_BIT(PORTD , PD4);

		if( ! (sec_units || sec_tens || min_units || min_tens || hour_units || hour_tens) )
		{
			SET_BIT(PORTD , PD0) ; /* turn on the Alarm */
		}
		else
		{
			if(sec_units != 0)
					{
						--sec_units ;
					}
					else
					{
						sec_units = 9 ;
						if(sec_tens != 0)
						{
							--sec_tens ;
						}
						else
						{
							sec_tens = 5 ;
							if(min_units != 0)
							{
								--min_units ;
							}
							else
							{
								min_units = 9 ;
								if(min_tens != 0)
								{
									--min_tens ;
								}
								else
								{
									min_tens = 5;
									if(hour_units != 0)
									{
										--hour_units ;
									}
									else
									{
										hour_units = 9 ;
										if(hour_tens != 0)
										{
											--hour_tens ;
										}

									}
								}
							}
						}
					}

				}
		}

}
/*************************************************************/
int main( void )
{
	DDRA  |=   0X3F  ; /* 6 output PINS */
	PORTA &= ~(0X3F) ; /* Initially all 7segments are disable */
	DDRC  |=   0X0F  ; /* 4 outputs PINS */
	PORTC &= ~(0X0F) ; /* Initial value is zero*/

	DDRD |=   0X31 ; /* PD0 , PD4 , PD5 are output pins */
	DDRD &= ~(0X0C); /* PD2 , PD3 are input pins */
	SET_BIT(PORTD , PD2) ; /* activate the internal pull up resistor for PD2 */

	DDRB  = 0X00 ; /* input port */
	PORTB = 0XFF ; /* activate the internal pull up resistor for the whole port */

	Timer_1_CTC_Mode_INIT( ) ;
	INT0_INIT( ) ;
	INT1_INIT( ) ;
	INT2_INIT( ) ;

	unsigned char Flag_count_mode_pressed = 1 ; /* indication for pressing on the button */
	unsigned char Flag_dec_Sec_pressed    = 1 ; /* indication for pressing on the button */
	unsigned char Flag_inc_Sec_pressed    = 1 ; /* indication for pressing on the button */
	unsigned char Flag_dec_min_pressed    = 1 ; /* indication for pressing on the button */
	unsigned char Flag_inc_min_pressed    = 1 ; /* indication for pressing on the button */
	unsigned char Flag_dec_hour_pressed   = 1 ; /* indication for pressing on the button */
	unsigned char Flag_inc_hour_pressed   = 1 ; /* indication for pressing on the button */

	for(;;)
	{
		if(! (PINB & 1<<PB7) )
		{
			_delay_ms(30) ; /* to avoid the debouncing phenomenon */
			if(! (PINB & 1<<PB7) )
			{
				UP_DW_bar =(Flag_count_mode_pressed)? !UP_DW_bar : UP_DW_bar ;
				Flag_count_mode_pressed = 0 ;
			}
		}
		else
		{
			Flag_count_mode_pressed = 1 ;
		}

		if(Timer_Falg)
		{
			update_state(UP_DW_bar) ;
			Timer_Falg = 0 ;
		}

		if( ! (PINB & 1<<PB6) )
		{
			_delay_ms(30) ;
			if( ! (PINB & 1<<PB6) )
			{
				if( Flag_inc_Sec_pressed )
				{
					Flag_inc_Sec_pressed = 0 ;
					if(sec_units != 9)
					{
						++sec_units ;
					}
					else if(sec_tens !=5 )
					{
						sec_units = 0 ;
						++sec_tens ;
					}
					else
					{
						/* no change for seconds = 59 */
					}
				}
			}

		}
		else
		{
			Flag_inc_Sec_pressed = 1 ;
		}

		if( ! (PINB & 1<<PB5) )
		{
			_delay_ms(30) ;
			if( ! (PINB & 1<<PB5) )
			{
				if( Flag_dec_Sec_pressed )
				{
					Flag_dec_Sec_pressed = 0 ;
					if(sec_units != 0)
					{
						--sec_units ;
					}
					else if(sec_tens != 0 )
					{
						sec_units = 9 ;
						--sec_tens ;
					}
					else
					{
						/* no change for seconds = 00 */
					}
				}
			}

		}
		else
		{
			Flag_dec_Sec_pressed = 1 ;
		}

		if( ! (PINB & 1<<PB4) )
		{
			_delay_ms(30) ;
			if( ! (PINB & 1<<PB4) )
			{
				if( Flag_inc_min_pressed )
				{
					Flag_inc_min_pressed = 0 ;
					if(min_units != 9)
					{
						++min_units ;
					}
					else if(min_tens !=5 )
					{
						min_units = 0 ;
						++min_tens ;
					}
					else
					{
						/* no change for minutes = 59 */
					}
				}
			}

		}
		else
		{
			Flag_inc_min_pressed = 1 ;
		}

		if( ! (PINB & 1<<PB3) )
		{
			_delay_ms(30) ;
			if( ! (PINB & 1<<PB3) )
			{
				if( Flag_dec_min_pressed )
				{
					Flag_dec_min_pressed = 0 ;
					if(min_units != 0)
					{
						--min_units ;
					}
					else if(min_tens != 0 )
					{
						min_units = 9 ;
						--min_tens ;
					}
					else
					{
						/* no change for minutes = 00 */
					}
				}
			}

		}
		else
		{
			Flag_dec_min_pressed = 1 ;
		}

		if( ! (PINB & 1<<PB1) )
		{
			_delay_ms(30) ;
			if( ! (PINB & 1<<PB1) )
			{
				if( Flag_inc_hour_pressed )
				{
					Flag_inc_hour_pressed = 0 ;
					if(hour_units != 9)
					{
						++hour_units ;
					}
					else if( hour_tens!=9 )
					{
						hour_units = 0 ;
						++hour_tens ;
					}
					else
					{
						// No action for hours = 99
					}
				}
			}

		}
		else
		{
			Flag_inc_hour_pressed = 1 ;
		}

		if( ! (PINB & 1<<PB0) )
		{
			_delay_ms(30) ;
			if( ! (PINB & 1<<PB0) )
			{
				if( Flag_dec_hour_pressed )
				{
					Flag_dec_hour_pressed = 0 ;
					if(hour_units != 0)
					{
						--hour_units ;
					}
					else if(hour_tens != 0)
					{
						--hour_tens  ;
						hour_units = 9 ;
					}
					else
					{
						// No action for hours = 00 ;
					}

				}
			}

		}
		else
		{
			Flag_dec_hour_pressed = 1 ;
		}


		PORTA = (PORTA & 0XC0) | (1<<PA5) ; /* Enable the First 7 segment  */
		PORTC = (PORTC & 0XF0) | (sec_units & 0X0F) ; /* Display units of seconds */
		_delay_ms(3) ; /* delay to notice the number */

		PORTA = (PORTA & 0XC0) | (1<<PA4) ; /* Enable the Second 7 segment  */
		PORTC = (PORTC & 0XF0) | (sec_tens & 0X0F) ; /* Display tens of seconds */
		_delay_ms(3) ; /* delay to notice the number */

		PORTA = (PORTA & 0XC0) | (1<<PA3) ; /* Enable the Third 7 segment  */
		PORTC = (PORTC & 0XF0) | (min_units & 0X0F) ; /* Display units of minutes */
		_delay_ms(3) ; /* delay to notice the number */

		PORTA = (PORTA & 0XC0) | (1<<PA2) ; /* Enable the fourth 7 segment  */
		PORTC = (PORTC & 0XF0) | (min_tens & 0X0F) ; /* Display tens of minutes */
		_delay_ms(3) ; /* delay to notice the number */

		PORTA = (PORTA & 0XC0) | (1<<PA1) ; /* Enable the Fifth 7 segment  */
		PORTC = (PORTC & 0XF0) | (hour_units & 0X0F) ; /* Display units of hours */
		_delay_ms(3) ; /* delay to notice the number */

		PORTA = (PORTA & 0XC0) | (1<<PA0) ; /* Enable the Sixth 7 segment  */
		PORTC = (PORTC & 0XF0) | (hour_tens & 0X0F) ; /* Display tens of hours */
		_delay_ms(3) ; /* delay to notice the number */

	}
	return 0 ;
}
