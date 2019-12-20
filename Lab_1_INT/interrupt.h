#pragma once
#include <msp430.h> 
#include <stdbool.h>

void clear_PORT1_intr_flag(int pin);

void clear_PORT2_intr_flag(int pin);

void turn_off_PORT1_intr(int pin);

void turn_off_PORT2_intr(int pin);

void turn_on_PORT1_intr(int pin, bool highSignal);

void turn_on_PORT2_intr(int pin, bool highSignal);

void init_PORT1_intr(int pin, bool highSignal);

void init_PORT2_intr(int pin, bool highSignal);

void refresh_PORT2_intr(int pin);
