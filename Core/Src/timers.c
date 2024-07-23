#include "timers.h"

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim6;

/**
 * @brief formula to calculate the overflow frequency of a timer
 * 
 * @param htim 
 * @return uint32_t 
 */
uint32_t tim_get_overflow_freq(TIM_HandleTypeDef *htim)
{
    uint16_t prescaler = htim->Instance->PSC;
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(htim);

    uint32_t APBx_freq = tim_get_APBx_freq(htim);

    return APBx_freq / ((prescaler + 1) * (period + 1));
}


/**
 * @brief TIM1 is a 16-bit timer used to capture the input clock pulses (input capture)
 * 
 */
void init_TIM1(void)
{
    /* Peripheral clock enable */
    __HAL_RCC_TIM1_CLK_ENABLE();

    /**TIM1 GPIO Configuration
    PA10     ------> TIM1_CH3
    */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = CLOCK_INPUT;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* TIM1 interrupt Init */
    HAL_NVIC_SetPriority(TIM1_CC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);

    TIM_SlaveConfigTypeDef sSlaveConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 1;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 65535;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_IC_Init(&htim1) != HAL_OK)
    {
        Error_Handler();
    }

    sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
    sSlaveConfig.InputTrigger = TIM_TS_ITR0;
    if (HAL_TIM_SlaveConfigSynchro(&htim1, &sSlaveConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING; // input is inverted
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    if (HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_3) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief TIM2 is a 32-bit timer used to generate the clock pulses
 * 
 */
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
    htim2.Init.Period = FREQUENCY;
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
 * @brief TIM6 used to trigger TRANSPORT_PPQN pin low after short delay
 * 
 */
void init_TIM6()
{
    /* Peripheral clock enable */
    __HAL_RCC_TIM6_CLK_ENABLE();
    
    /* TIM6 interrupt Init */
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);

    TIM_MasterConfigTypeDef sMasterConfig = {0};

    // 200 BPM = 3.33 BPS, so 3.33 / 24 = 0.13888888888 pulses per second
    // 
    // 48000000 / ((500 + 1) * (32000 + 1)) == 2.99Hz

    htim6.Instance = TIM6;
    htim6.Init.Prescaler = 10;
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.Period = 25000;
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    
    if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief this callback needs to trigger at a rate of PPQN * 2
 * On even it will advance PPQN by 1 and write PPQN_96 pin HIGH
 * On odd it will write PPQN_96 pin LOW
 * When PULSE == 0, Increments Step + 1 and writes PPQN_1 pin HIGH
 * After 4 pulse counts, writes PPQN_1 pin LOW
 *
 * @param htim
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        ok_clock_advance();
    }
    else if (htim->Instance == TIM6)
    {
        HAL_GPIO_WritePin(GPIOA, TRANSPORT_PPQN, LOW); // set to low after short delay (delay dertmined by TIM6)
        HAL_TIM_Base_Stop_IT(htim);
    }
}

/**
 * @brief Input Capture Callback for all TIMx configured in Input Capture mode
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        ok_clock_capture();
        // this is working, but the problem now is that we need to handle missed PPQNs,
        // and in order to do that we usually just trigger all remaining PPQNs instantly,
        // which will no longer work because the connected modules 'might' not be configured to handle
        // trigger events so rapidly.
        // We would have to execute the remaining PPQNs over a short period of time, so the rise and fall of
        // the signal gets detected by connected modules.

        // alternatively, the external devices could be setup so that they listen for the quarter note signal,
        // and then determine if they have executed all their PPQNs, at which point they would execute them

        // or, the F4 is actually running at twice the speed, so take the new
        // rate at which to trigger pulse out on and off === (numTicksPerPulse / (PPQN - 1 - PULSE)) * 2

        // ACTUALLY, your pulse is actually PPQN * 2, so you would need to multiply the above equation by 2.

        // or, maybe you could set the new TIM2 speed to the above formula

        // or, external modules stop their clocks once their pulse === PPQN - 1. They don't start until quarter note comes in
        // if quarter note comes early, they rapidly execute remaining pulses via a qeuee
        // if you do this, then PPQN on clock means nothing
    }
}

/**
 * @brief This function handles TIM1 capture compare interrupt.
 */
void TIM1_CC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim1);
}

/**
 * @brief This function handles TIM2 global interrupt.
 */
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

/**
 * @brief This function handles TIM6 global and DAC underrun error interrupts.
 */
void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim6);
}