#pragma once

#include "main.h"

#define CLOCK_SOURCE_INTERNAL 0
#define CLOCK_SOURCE_EXTERNAL 2
#define CLOCK_SOURCE_MIDI 1

extern int PPQN;
extern uint16_t PULSE;
extern bool ODD_PULSE;
extern bool SYNCHRONIZE;
extern int CLOCK_SOURCE;
extern int encoderDirection;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;

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