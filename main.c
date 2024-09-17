#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
/*************************************************************/
#define SET_BIT(REG , BIT)      ( REG |=  (1 << (BIT)) )
#define CLR_BIT(REG , BIT)      ( REG &= ~(1 << (BIT)) )
#define TOG_BIT(REG , BIT)      ( REG ^=  (1 << (BIT)) )
#define GET_BIT(REG , BIT)      ( (REG >> BIT) & 0X01  )
/*************************************************************/
unsigned char Seconds  = 0 ; /* Legal Values [0:59] */
unsigned char Minutes  = 0 ; /* Legal Values [0:59] */
unsigned char Hours    = 0 ; /* Legal Values [0:23] */
unsigned char UP_DW_bar  = 1 ; /* To differentiate between the counting modes */
volatile unsigned char Timer_Falg = 0 ; /* indicated for triggered interrupt */
/*************************************************************/
void Timer_1_CTC_Mode_INIT( void )
{
	SREG  |= 1<<7 ; /* Global Interrupt Enable */
	TCCR1A = (1<<FOC1A) | (1<<FOC1B) ; /* non PWM mode */
	TCCR1B = (1<<WGM12) | (1<<CS10) | (1<<CS12) ; /* 1024 Prescaler */
	TCNT1  = 0 ; /* start counting from zero */
	OCR1A  = 15625 ; /* Number of counts required for the timer to trigger an interrupt every one second */
	SET_BIT(TIMSK , OCIE1A);  /* Module interrupt Enable */

}
/*************************************************************/
ISR(TIMER1_COMPA_vect)
{
	Timer_Falg = 1 ; /* simple ISR to avoid a huge interrupts latency */
}
/*************************************************************/
void INT0_INIT( void )
{
	SET_BIT(MCUCR , ISC01);  /* interrupt to be triggered with falling edge */
	SET_BIT(GICR  , INT0);  /* module interrupt Enable */
}

/*************************************************************/
ISR(INT0_vect)
{
	Hours   = 0 ;
	Minutes = 0 ;
	Seconds = 0 ;

	CLR_BIT(PORTD , PD0) ; /* to clear the buzzer press on reset */
	UP_DW_bar  = 1 ; /* start count up again */

}
/*************************************************************/
void INT1_INIT( void )
{
	MCUCR |= (1<<ISC11) | (1<<ISC10) ; /* interrupt to be triggered with rising edge */
	SET_BIT(GICR , INT1) ; /* module interrupt Enable */
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
	SET_BIT(GICR  , INT2)  ; /* module interrupt Enable */

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
	if (UP_DW_bar)
	{
	    SET_BIT(PORTD, PD4);
	    CLR_BIT(PORTD, PD5);

	    ++Seconds ;
	    if( Seconds == 60 )
	    {
	    	Seconds = 0 ;
	    	++Minutes ;
	    	if( Minutes == 60 )
	    	{
	    		Minutes = 0 ;
	    		++Hours;
	    		if( Hours == 24 )
	    		{
	    			Seconds = 0 ;
	    			Minutes = 0 ;
	    			Hours = 0 ;
	    		}
	    	}
	    }
	}

	else
	{
		SET_BIT(PORTD , PD5);
		CLR_BIT(PORTD , PD4);

		if( !(Seconds || Minutes || Hours) )
		{
			SET_BIT(PORTD , PD0); /* turn on the Alarm */
		}
		else
		{
			if( Seconds > 0 )
				--Seconds;
			else
			{
				if( Minutes > 0 )
				{
					Seconds = 59;  // Reset Seconds
					--Minutes;
				}
				else
				{
					if( Hours > 0 )
					{
						Seconds = 59;
						Minutes = 59;
						--Hours;
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

volatile unsigned char Flag_count_mode_pressed = 1 ; /* indication for pressing on the button */
volatile unsigned char Flag_dec_Sec_pressed    = 1 ; /* indication for pressing on the button */
volatile unsigned char Flag_inc_Sec_pressed    = 1 ; /* indication for pressing on the button */
volatile unsigned char Flag_dec_min_pressed    = 1 ; /* indication for pressing on the button */
volatile unsigned char Flag_inc_min_pressed    = 1 ; /* indication for pressing on the button */
volatile unsigned char Flag_dec_hour_pressed   = 1 ; /* indication for pressing on the button */
volatile unsigned char Flag_inc_hour_pressed   = 1 ; /* indication for pressing on the button */

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
					if(Seconds < 59)
					{
						++Seconds ;
					}

					else
					{
						/* no change for Seconds = 59 */
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
					if(Seconds > 0)
					{
						--Seconds ;
					}
					else
					{
						/* no change for Seconds = 00 */
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
					if(Minutes < 59)
					{
						++Minutes ;
					}
					else
					{
						/* no change for Minutes = 59 */
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
					if(Minutes > 0)
					{
						--Minutes ;
					}
					else
					{
						/* no change for Minutes = 00 */
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
					if(Hours < 99)
					{
						++Hours ;
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
					if(Hours > 0)
					{
						--Hours ;
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
		PORTC = (PORTC & 0XF0) | ( (Seconds%10) & 0X0F ) ; /* Display units of Seconds */
		_delay_ms(2) ; /* delay to notice the number */


		PORTA = (PORTA & 0XC0) | (1<<PA4) ; /* Enable the Second 7 segment  */
		PORTC = (PORTC & 0XF0) | ( (Seconds/10) & 0X0F) ; /* Display tens of Seconds */
		_delay_ms(2) ; /* delay to notice the number */


		PORTA = (PORTA & 0XC0) | (1<<PA3) ; /* Enable the Third 7 segment  */
		PORTC = (PORTC & 0XF0) | ( (Minutes%10) & 0X0F) ; /* Display units of Minutes */
		_delay_ms(2) ; /* delay to notice the number */


		PORTA = (PORTA & 0XC0) | (1<<PA2) ; /* Enable the fourth 7 segment  */
		PORTC = (PORTC & 0XF0) | ( (Minutes/10) & 0X0F) ; /* Display tens of Minutes */
		_delay_ms(2) ; /* delay to notice the number */


		PORTA = (PORTA & 0XC0) | (1<<PA1) ; /* Enable the Fifth 7 segment  */
		PORTC = (PORTC & 0XF0) | ( (Hours%10) & 0X0F) ; /* Display units of hours */
		_delay_ms(2) ; /* delay to notice the number */


		PORTA = (PORTA & 0XC0) | (1<<PA0) ; /* Enable the Sixth 7 segment  */
		PORTC = (PORTC & 0XF0) | ( (Hours/10) & 0X0F) ; /* Display tens of hours */
		_delay_ms(2) ; /* delay to notice the number */

	}
	return 0 ;
}
