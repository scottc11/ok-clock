#include "timers.h"

TIM_HandleTypeDef htim6;

// what is the frequency in Hz of a quarter note at a rate of 200 BPM
// now divide that by 24, and then divide again by 2, and you have your down pulse frequency
// or just set it to 3 hz...


uint32_t tim_get_overflow_freq(TIM_HandleTypeDef *htim)
{
    uint16_t prescaler = htim->Instance->PSC;
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(htim);

    uint32_t APBx_freq = tim_get_APBx_freq(htim);

    return APBx_freq / ((prescaler + 1) * (period + 1));
}

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
    htim6.Init.Prescaler = 20;
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.Period = 32000;
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
 * @brief This function handles TIM6 global and DAC underrun error interrupts.
 */
void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim6);
}