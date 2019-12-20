#pragma once
#include <msp430.h> 
#include <stdbool.h>

void init_PORT1_in(int pin);

void init_PORT2_in(int pin);

void init_PORT8_out(int pin);

bool get_PORT1_in_state(int pin);

bool get_PORT2_in_state(int pin);

bool get_PORT8_out_state(int pin);

void set_PORT8_out_state(int pin, bool state);

void debounce_delay(int cycles);
