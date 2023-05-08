#pragma once

#include "main.h"

extern int encoderDirection;

void ok_clock_init();
void ok_clock_loop();
void ok_clock_set_overflow();

void encoderISR_A();
void encoderISR_B();