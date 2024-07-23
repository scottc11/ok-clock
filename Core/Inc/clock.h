#pragma once

#include "main.h"

extern TIM_HandleTypeDef htim1; // defined in timers.c
extern TIM_HandleTypeDef htim2; // defined in timers.c
extern TIM_HandleTypeDef htim6; // defined in timers.c

extern uint32_t FREQUENCY;
extern uint16_t PULSE;
extern int CLOCK_SOURCE;
extern int encoderDirection;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;

void ok_clock_init();
void ok_clock_capture();

void ok_clock_start();
void ok_clock_stop();
void ok_clock_reset();
void ok_clock_advance();

void ok_clock_set_frequency(uint32_t frequency);
void ok_clock_set_clock_source(int clock_source);

void encoder_handle_rotation();