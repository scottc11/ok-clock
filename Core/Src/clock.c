#include "clock.h"

TIM_HandleTypeDef htim2;

uint16_t PERIOD = 3000;

const int PPQN = 192; // 96, but we need to have a clock LOW period for output pins so we double it
int STEPS_PER_BAR = 4;

// Define the initial state of the encoder pins
int encoderStateA = 0;
int encoderStateB = 0;

// Define the previous state of the encoder pins
int prevEncoderStateA = 0;
int prevEncoderStateB = 0;

// Define a variable to store the direction of the encoder rotation
int encoderDirection = 0;

void ok_clock_init()
{
    // initialize stuff
    init_TIM2();
    HAL_TIM_Base_Start_IT(&htim2);
}

void ok_clock_loop()
{

}

void ok_clock_set_period() {
    int incrementAmount = 100;

    if (encoderDirection == 1)
    {
        PERIOD -= incrementAmount;
    }
    else if (encoderDirection == -1)
    {
        PERIOD += incrementAmount;
    }

    if (PERIOD > 250 && PERIOD < 60000)
    {
        if (__HAL_TIM_GET_COUNTER(&htim2) >= PERIOD)
        {
            __HAL_TIM_SetCounter(&htim2, PERIOD - incrementAmount);
        }

        __HAL_TIM_SetAutoreload(&htim2, PERIOD);
    }
}

void init_TIM2(void)
{
    
    __HAL_RCC_TIM2_CLK_ENABLE();

    HAL_NVIC_SetPriority(TIM2_IRQn, 2, 1);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 20;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = PERIOD;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief This function handles TIM2 global interrupt.
 */
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

int ODD_PULSE = 0; // sudo bool
uint16_t PULSE = 0;
uint8_t STEP = 0;

/**
 * @brief this callback needs to trigger at a rate of PPQN * 2
 * On even it will advance PPQN by 1 and write PPQN_96 pin HIGH
 * On odd it will write PPQN_96 pin LOW
 * When PULSE == 0, Increments Step + 1 and writes PPQN_1 pin HIGH
 * After 4 pulse counds, writes PPQN_1 pin LOW
 *
 * @param htim
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        // __HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);
        // do clock stuff

        if (ODD_PULSE == 0) {
            HAL_GPIO_WritePin(GPIOA, TRANSPORT_PPQN_96, 1);
            ODD_PULSE = 1;
        } else {
            HAL_GPIO_WritePin(GPIOA, TRANSPORT_PPQN_96, 0);
            ODD_PULSE = 0;
        }

        if (PULSE == 0)
        {
            HAL_GPIO_WritePin(GPIOA, TRANSPORT_PPQN_1, 1);
            HAL_GPIO_WritePin(GPIOA, RESET_BTN_LED, 1);
        }

        if (PULSE == 12)
        {
            HAL_GPIO_WritePin(GPIOA, TRANSPORT_PPQN_1, 0);
            HAL_GPIO_WritePin(GPIOA, RESET_BTN_LED, 0);
        }

        if (PULSE < PPQN - 1) {
            PULSE++;
        } else {
            PULSE = 0;
            
            if (STEP < STEPS_PER_BAR - 1) {
                STEP++;
            } else {
                STEP = 0;
            }
        }
    }
}

// Interrupt service routine for encoder pin A
void encoderISR_A()
{
    encoderStateA = HAL_GPIO_ReadPin(GPIOA, ENC_CHAN_A);

    if (encoderStateA != prevEncoderStateA)
    {
        // If the A pin changed, read the B pin to determine direction
        if (encoderStateA == encoderStateB)
        {
            encoderDirection = -1;
            ok_clock_set_period();
        }
        else
        {
            encoderDirection = 1;
            ok_clock_set_period();
        }
    }

    prevEncoderStateA = encoderStateA;
}

// Interrupt service routine for encoder pin B
void encoderISR_B()
{
    encoderStateB = HAL_GPIO_ReadPin(GPIOA, ENC_CHAN_B);

    if (encoderStateB != prevEncoderStateB)
    {
        // If the B pin changed, read the A pin to determine direction
        if (encoderStateB == encoderStateA)
        {
            encoderDirection = 1;
            // ok_clock_set_period();
        }
        else
        {
            encoderDirection = -1;
            // ok_clock_set_period();
        }
    }

    prevEncoderStateB = encoderStateB;
}