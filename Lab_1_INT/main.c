#include "ports.h"
#include "interrupt.h"

#define FIRST_BUTTON_PIN BIT7
#define SECOND_BUTTON_PIN BIT2

#define FIRST_LED_PIN BIT1
#define SECOND_LED_PIN BIT2

#define BUTTON_PRESSED_STATE false

void turnOnFirstLed()
{
    set_PORT8_out_state(FIRST_LED_PIN, true);
}

void turnOnSecondLed()
{
    set_PORT8_out_state(SECOND_LED_PIN, true);
}

void turnOffFirstLed()
{
    set_PORT8_out_state(FIRST_LED_PIN, false);
}

void turnOffSecondLed()
{
    set_PORT8_out_state(SECOND_LED_PIN, false);
}

bool isFirstButtonPressed()
{
    return get_PORT1_in_state(FIRST_BUTTON_PIN) == BUTTON_PRESSED_STATE;
}

bool isSecondButtonPressed()
{
    return get_PORT2_in_state(SECOND_BUTTON_PIN) == BUTTON_PRESSED_STATE; //!
}

bool isFirstLedActive()
{
    return get_PORT8_out_state(FIRST_LED_PIN);
}

bool isSecondLedActive()
{
    return get_PORT8_out_state(SECOND_LED_PIN);
}

void init_ports()
{
    //setup pins for buttons
	init_PORT1_in(FIRST_BUTTON_PIN);
	init_PORT2_in(SECOND_BUTTON_PIN);

	//setup pins for leds
	init_PORT8_out(FIRST_LED_PIN);
	init_PORT8_out(SECOND_LED_PIN);

	//setup interrupts
	init_PORT1_intr(FIRST_BUTTON_PIN, true);
	init_PORT2_intr(SECOND_BUTTON_PIN, true);
}

int s1 = 0, s2 = 0;

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	
    __bis_SR_register(GIE);


    init_ports();

    while(true)
	{
	    __no_operation();
	}

	return 0;
}

#pragma vector = PORT1_VECTOR
__interrupt void P1INT()
{
    turn_off_PORT1_intr(FIRST_BUTTON_PIN);
    debounce_delay(5000);
    clear_PORT1_intr_flag(FIRST_BUTTON_PIN);

    if(isFirstLedActive())
    {
    	turnOffFirstLed();
    }
    else if(!isSecondButtonPressed())
    {
    	if(!isSecondLedActive())
    	{
    		turnOnSecondLed();
    		turn_on_PORT1_intr(FIRST_BUTTON_PIN, false);

    		return;
    	}
    	else
    	{
    		turnOffSecondLed();
    		turn_on_PORT1_intr(FIRST_BUTTON_PIN, true);

    		return;
    	}
    }

    turn_on_PORT1_intr(FIRST_BUTTON_PIN, true);
}

#pragma vector = PORT2_VECTOR
__interrupt void P2INT()
{
    turn_off_PORT2_intr(SECOND_BUTTON_PIN);
    debounce_delay(1000);
    clear_PORT2_intr_flag(SECOND_BUTTON_PIN);

	debounce_delay(1000);

	if(!s2)
	{
		s2 = 1;
		turn_on_PORT2_intr(SECOND_BUTTON_PIN, false);
	}
	else
	{
		if(isFirstButtonPressed())
		{
			turnOnFirstLed();
			s2 = 0;
			turn_on_PORT2_intr(SECOND_BUTTON_PIN, true);
		}
	}

	refresh_PORT2_intr(SECOND_BUTTON_PIN);
}
