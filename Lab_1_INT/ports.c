#include "ports.h"

#define LOW_PORT_SIGNAL 0x00

void init_PORT1_in(int pin)
{
	P1DIR &= ~pin;
	P1REN |= pin;
	P1OUT |= pin;
}

void init_PORT2_in(int pin)
{
	P2DIR &= ~pin;
	P2REN |= pin;
	P2OUT |= pin;
}

void init_PORT8_out(int pin)
{
	P8DIR |= pin;
	P8OUT &= ~ pin;
}

bool get_PORT1_in_state(int pin)
{
    return (P1IN&pin) != LOW_PORT_SIGNAL;
}

bool get_PORT2_in_state(int pin)
{
    return (P2IN & pin) != LOW_PORT_SIGNAL;
}

bool get_PORT8_out_state(int pin)
{
    return (P8OUT & pin) != LOW_PORT_SIGNAL;
}

void set_PORT8_out_state(int pin, bool state)
{
	if(state)
	{
		P8OUT |= pin;
	}
	else 
	{
	    P8OUT &= ~pin;
	}
}

void debounce_delay(int cycles)
{
    volatile int indexer = 0; 
    for(; indexer < cycles; indexer ++ )
    	indexer++;
}
