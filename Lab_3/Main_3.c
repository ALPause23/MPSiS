#include <msp430.h>

#define TA1   0
#define WDT   1

volatile int timer_mode = 0; // 0 - TA
                             // 1 - WDT
volatile int count_timer;

volatile int led1_on = 0;
volatile int led2_on = 0;
volatile int led3_on = 0;


void set_Timer(int a)
{
    switch(a)
    {
        case 0:
            TA1CCR0 = 10485;
            TA1EX0 = TAIDEX_4;
            TA1CTL = TASSEL_2 | ID_1 | MC_1 | TACLR;
            P8OUT &= ~BIT1;
            break;
        case 1:
            WDTCTL = WDTPW | WDTTMSEL | WDTCNTCL | WDTIS_4 | WDTSSEL_0;
            P8OUT |= BIT1;
            break;
    }
}

void start_TimerA1()
{
    set_Timer(TA1);
    TA1CCTL0 = CCIE;
}

void start_WDT(void)
{
	SFRIE1 |=  WDTIE;
    set_Timer(WDT);

}

void _stop_TimerA1(void)
{
	TA1CTL |= MC_0;
    TA1CCTL0 &= ~CCIE;
}

void _stop_WDT(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    SFRIE1 &= ~WDTIE;
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    P8OUT &= ~BIT1;///////////////////////////////////////////////////////////////////////////
}

void ledOnOff_TA1(void)
{
    count_timer++;

    if(P1IN & BIT7)
    {
        switch(count_timer)
        {
        case 6:
        	led3_on = 0;
            P1OUT &= ~BIT3;
            break;
        case 12:
        	led2_on = 0;
            P1OUT &= ~BIT2;
            break;
        case 18:
        	led1_on = 0;
            P1OUT &= ~BIT1;
            _stop_TimerA1();
            break;
        }
    }
    else
    {
        switch(count_timer)
        {
        case 12:
        	led1_on = 1;
            P1OUT |= BIT1;
            break;
        case 24:
        	led2_on = 1;
            P1OUT |= BIT2;
            break;
        case 36:
        	led3_on = 1;
        	P1OUT |= BIT3;
        	count_timer = 0;
        	_stop_TimerA1();
        	break;
        }
    }
}

void ledOnOff_WDT(void)
{
    count_timer++;

    if(P1IN & BIT7)
    {
        switch(count_timer)
        {
        case 19:
        	led3_on = 0;
            P1OUT &= ~BIT3;
            break;
        case 38:
        	led2_on = 0;
            P1OUT &= ~BIT2;
            break;
        case 57:
        	led1_on = 0;
            P1OUT &= ~BIT1;
            _stop_WDT();
            break;
        }
    }
    else
    {
        switch(count_timer)
        {

        case 38:
        	led1_on = 1;
            P1OUT |= BIT1;
            break;
        case 76:
        	led2_on = 1;
            P1OUT |= BIT2;
            break;
        case 114:
        	led3_on = 1;
        	P1OUT |= BIT3;
        	count_timer = 0;
        	_stop_WDT();
        	break;
        }
    }
}

void delay(int j)
{
    volatile i, k = 1;
    for (i = 0; i < j; i++)
    {
        k++;
    }
}

void init_INT()
{
    P1IE |= BIT7;
    P1IES |= BIT7;
    P1IFG &= ~BIT7;

    P2IE |= BIT2;
    P2IES |= BIT2;
    P2IFG &= ~BIT2;
}

void init_button_S1_S2_out()
{
    P1DIR &= ~BIT7;
    P2DIR &= ~BIT2;

    P1REN |= BIT7;
    P2REN |= BIT2;

    P1OUT |= BIT7;
    P2OUT |= BIT2;

    init_INT();
}

void init_LED(void)
{
	P8DIR |= BIT1;
	P8OUT &= ~BIT1;

    P1DIR |= BIT1;
    P1DIR |= BIT2;
    P1DIR |= BIT3;

    P1OUT &= ~BIT1;
    P1OUT &= ~BIT2;
    P1OUT &= ~BIT3;
}

void init()
{
    P1DIR |= BIT4;
    P1OUT |= BIT4;
    P1SEL |= BIT4;

    init_LED();

    init_button_S1_S2_out();

    __bis_SR_register(GIE);
}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	init();
	
	UCSCTL4 |= SELA__REFOCLK;

	TA0CCTL3 = OUTMOD_2;
	TA0CTL = TASSEL_1 | ID_0 | MC_3 | TACLR;
	TA0CCR0 = 32768;
	TA0CCR3 = 8192;
	__no_operation();
	return 0;
}

#pragma vector = PORT1_VECTOR
__interrupt void int1(void)
{
    delay(2000);

    count_timer = 0;

    if(P1IES & BIT7)
    {
    	if(led3_on == 1) {
    		count_timer += timer_mode ? 38 : 12;
    	}
    	if(led2_on == 1) {
    	    count_timer += timer_mode ? 38 : 12;
    	}
    	if(led1_on == 1) {
    	   count_timer += timer_mode ? 38 : 12;
    	}

        if(timer_mode) {
            start_WDT();
        }
        else
        	start_TimerA1();
    }
    else
    {
    	if(led3_on == 0) {
    		count_timer += timer_mode ? 19 : 6;
    	}
    	if(led2_on == 0) {
    	    count_timer += timer_mode ? 19 : 6;
    	}
    	if(led1_on == 0) {
    	    count_timer += timer_mode ? 19 : 6;
    	}

        if(timer_mode)
        {
            start_WDT();
        }
        else start_TimerA1();
    }
    P1IES ^= BIT7;
    P1IFG &= ~BIT7;
}

#pragma vector = PORT2_VECTOR
__interrupt void int2 (void)
{
    delay(1000);

    if(timer_mode) // status set now - WDT
    {
        _stop_WDT();
        start_TimerA1();

        timer_mode = 0;
    }
    else  // status set now - TA1
    {
    	set_Timer(TA1);
    	//_stop_TimerA1();
        set_Timer(WDT);
        //start_WDT();

        timer_mode = 1;
    }

    P2IFG &= ~BIT2;
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA_1(void)
{
    ledOnOff_TA1();
}

#pragma vector = WDT_VECTOR
__interrupt void WDT_(void)
{
    ledOnOff_WDT();
}
