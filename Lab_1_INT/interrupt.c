#include "interrupt.h"

void clear_PORT1_intr_flag(int pin)
{
    P1IFG &= ~pin;
}

void clear_PORT2_intr_flag(int pin)
{
    P2IFG &= ~pin;
}

void turn_off_PORT1_intr(int pin)
{
    P1IE &= ~pin;
}

void turn_off_PORT2_intr(int pin)
{
    P1IE &= ~pin;
}

void turn_on_PORT1_intr(int pin, bool highSignal)
{
    P1IE |= pin;
    
    if(highSignal)
    {
    	P1IES |= pin;
    }
    else
    {
        P1IES &= ~pin;
    }
}

void turn_on_PORT2_intr(int pin, bool highSignal)
{
    P2IE |= pin;
    
    if(highSignal)
    {
    	P2IES |= pin;
    }
    else
    {
        P2IES &= ~pin;
    }
}

void init_PORT1_intr(int pin, bool highSignal)
{
    clear_PORT1_intr_flag(pin);
    turn_on_PORT1_intr(pin, highSignal);
}

void init_PORT2_intr(int pin, bool highSignal)
{
    clear_PORT2_intr_flag(pin);
    turn_on_PORT2_intr(pin, highSignal);
}

void refresh_PORT2_intr(int pin)
{
	P2IE |= pin;
}
