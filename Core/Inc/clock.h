#pragma once

#include "main.h"

extern int encoderDirection;

void ok_clock_init();
void ok_clock_loop();
void clock_reset();

// void TIM2_IRQHandler(void);
// void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void init_TIM2();

void encoderISR_A();
void encoderISR_B();