/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

typedef int bool;
#define true 1
#define false 0

#define HIGH 1
#define LOW 0

#define RESET_BTN GPIO_PIN_8
#define RESET_BTN_LED GPIO_PIN_15

#define ENC_BTN GPIO_PIN_7
#define ENC_CHAN_A GPIO_PIN_6
#define ENC_CHAN_B GPIO_PIN_5

#define TOGGLE_SWITCH GPIO_PIN_9

#define CLOCK_INPUT GPIO_PIN_10
#define CLOCK_OUTPUT GPIO_PIN_11
#define CLOCK_RESET_OUTPUT GPIO_PIN_12

#define TRANSPORT_PPQN_1 GPIO_PIN_1
#define TRANSPORT_PPQN_96 GPIO_PIN_0
#define TRANSPORT_RESET GPIO_PIN_2
#define TRANSPORT_BAR_RESET GPIO_PIN_3

extern bool ODD_PULSE;
extern uint16_t PULSE;
extern uint8_t STEP;
extern bool encoderIsPressed;


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
