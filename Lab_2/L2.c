#include <msp430.h>
#include <stdbool.h>

volatile int s1_click = 0;
volatile int s2_click = 0;

void debounce_delay(int cycles)
{
    volatile int indexer = 0;
    for(; indexer < cycles; indexer ++ )
    	indexer++;
}

void init_port1_2_out()
{
	P1DIR &= ~BIT7;
	P2DIR &= ~BIT2;

	P1REN |= BIT7;
	P2REN |= BIT2;

	P1OUT |= BIT7;
	P2OUT |= BIT2;
}


void init_INT()
{
	P1IE |= BIT7;
	P1IES |= BIT7;
	P1IFG &= ~BIT7;

	P2IE |= BIT2;
	P2IES |= BIT2;
	P2IFG &= ~BIT7;
}

void initialize()
{
	__bis_SR_register(GIE);
	init_port1_2_out();
	init_INT();

	P7DIR |= BIT7;
    P7SEL |= BIT7; // peripherial
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    initialize();

	UCSCTL3 = SELREF__REFOCLK;
    UCSCTL1 = DCORSEL_0;

	UCSCTL2 = 16; // N = 16; D = 1 (#define FLLD__1 == (0x0000); #define FLLN4 == (0x0010))
    UCSCTL3 |= FLLREFDIV__4;
    UCSCTL5 |= DIVM__16;
    UCSCTL4 = SELM__DCOCLK;

	__no_operation();
	return 0;
}

#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
	debounce_delay(1000);
//	if(P1IN&BIT7 == 0)
//	{
		if (s1_click)
		{
			s1_click = 0;
		    _bic_SR_register_on_exit(LPM2_bits);

		}
		else
		{
			s1_click = 1;
			_bis_SR_register_on_exit(LPM2_bits);
		}
//	}
	debounce_delay(1000);
	P1IFG &= ~BIT7;
}


#pragma vector = PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
	debounce_delay(1000);
	
	if (s2_click)
	{
		s2_click = 0;
		UCSCTL4 = SELM__DCOCLK;  // 32,768 kHz
		UCSCTL5 = DIVM__16;

	} else {
		s2_click = 1;
		UCSCTL4 = SELM__VLOCLK; 	// 9,4 kHz
		UCSCTL5 = DIVM__2;

	}
	debounce_delay(1000);

    P2IFG &= ~BIT2;
}
