#pragma once

#include "main.h"

#define CLOCK_SOURCE_INTERNAL 0
#define CLOCK_SOURCE_EXTERNAL 1
#define CLOCK_SOURCE_MIDI 2

extern int CLOCK_SOURCE;
extern int encoderDirection;

void ok_clock_init();
void ok_clock_advance();
void ok_clock_capture();
void ok_clock_reset();
void ok_clock_set_frequency(uint32_t frequency);
void ok_clock_set_clock_source(int clock_source);

// void TIM2_IRQHandler(void);
// void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void init_TIM1();
void init_TIM2();

void encoderISR_A();
void encoderISR_B();