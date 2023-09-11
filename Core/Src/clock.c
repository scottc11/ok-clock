#include "clock.h"

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

uint16_t FREQUENCY = 3000;

bool encoderIsPressed = false;

bool ODD_PULSE = false;
uint16_t PULSE = 0;
uint8_t STEP = 0;

const int PPQN = 192; // 96, but we need to have a clock LOW period for output pins so we double it
int STEPS_PER_BAR = 4;

// Define the initial state of the encoder pins
int encoderStateA = 0;
int encoderStateB = 0;

// Define the previous state of the encoder pins
int prevEncoderStateA = 0;
int prevEncoderStateB = 0;

int CLOCK_SOURCE = CLOCK_SOURCE_INTERNAL; // internal, external, or MIDI (0, 1, 2)

// Define a variable to store the direction of the encoder rotation
int encoderDirection = 0;

void ok_clock_init()
{
    // initialize stuff
    init_TIM2();
    init_TIM1();
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_3);
    HAL_TIM_Base_Start_IT(&htim2);
    ok_clock_set_clock_source(HAL_GPIO_ReadPin(GPIOA, TOGGLE_SWITCH));
}

void ok_clock_advance()
{
    if (ODD_PULSE == false)
    {
        HAL_GPIO_WritePin(GPIOA, TRANSPORT_PPQN_96, HIGH);
        ODD_PULSE = true;
    }
    else
    {
        HAL_GPIO_WritePin(GPIOA, TRANSPORT_PPQN_96, LOW);
        ODD_PULSE = false;
    }

    if (PULSE == 0)
    {
        HAL_GPIO_WritePin(GPIOA, CLOCK_OUTPUT, LOW); // gate outs are inverted
        HAL_GPIO_WritePin(GPIOA, TRANSPORT_PPQN_1, HIGH);
        HAL_GPIO_WritePin(GPIOA, RESET_BTN_LED, HIGH);
    }

    if (PULSE == 12)
    {
        HAL_GPIO_WritePin(GPIOA, CLOCK_RESET_OUTPUT, HIGH); // inverted
        HAL_GPIO_WritePin(GPIOA, TRANSPORT_RESET, LOW);
        HAL_GPIO_WritePin(GPIOA, CLOCK_OUTPUT, HIGH); // inverted
        HAL_GPIO_WritePin(GPIOA, TRANSPORT_PPQN_1, LOW);
        HAL_GPIO_WritePin(GPIOA, RESET_BTN_LED, LOW);
    }

    if (PULSE < PPQN - 1)
    {
        PULSE++;
    }
    else
    {
        if (CLOCK_SOURCE == CLOCK_SOURCE_EXTERNAL) {
            // external input will reset pulse to 0 and resume TIM4 in input capture callback
            __HAL_TIM_DISABLE(&htim2); // halt TIM2 until a new input capture event occurs
        } else {
            PULSE = 0;

            if (STEP < STEPS_PER_BAR - 1) // I don't think steps matter any more...
            {
                STEP++;
            }
            else
            {
                STEP = 0;
            }
        }
    }
}

/**
 * @brief this gets triggered when an Input Capture event occurs. 
 * 
 */
void ok_clock_capture()
{
    // almost always, there will need to be at least 1 pulse not yet executed prior to an input capture, so you must trigger all remaining pulses
    if (PULSE < PPQN - 1)
    {
        // handleStep();
    }

    __HAL_TIM_SetCounter(&htim1, 0); // reset after each input capture
    __HAL_TIM_SetCounter(&htim2, 0); // reset after each input capture
    __HAL_TIM_ENABLE(&htim2);        // re-enable TIM2 (it gets disabled should the pulse count overtake PPQN before a new input capture event occurs)
    uint32_t inputCapture = __HAL_TIM_GetCompare(&htim1, TIM_CHANNEL_3);
    ok_clock_set_frequency(inputCapture / PPQN);
    PULSE = 0;
    ODD_PULSE = false;
    ok_clock_advance();
}

void ok_clock_reset()
{
    PULSE = 0;
    ODD_PULSE = false;
    STEP = 0;
    HAL_GPIO_WritePin(GPIOA, CLOCK_RESET_OUTPUT, LOW); // inverted
    HAL_GPIO_WritePin(GPIOA, TRANSPORT_RESET, HIGH);
}

void ok_clock_set_clock_source(int clock_source)
{
    switch (clock_source)
    {
    case CLOCK_SOURCE_INTERNAL:
        CLOCK_SOURCE = CLOCK_SOURCE_INTERNAL;
        HAL_NVIC_DisableIRQ(TIM1_CC_IRQn);
        __HAL_TIM_ENABLE(&htim2); // re-enable TIM4 (it gets disabled should the pulse count overtake PPQN before a new input capture event occurs)
        break;
    case CLOCK_SOURCE_EXTERNAL:
        CLOCK_SOURCE = CLOCK_SOURCE_EXTERNAL;
        HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
        break;
    case CLOCK_SOURCE_MIDI:
        /* code */
        break;
    }
}

/**
 * @brief Sets the tempo by setting the Timers overflow / reload value
 * 
 * @param frequency 
 */
void ok_clock_set_frequency(uint32_t frequency)
{
    if (frequency > 250 && frequency < 60000)
    {
        FREQUENCY = frequency;

        // this block might not be needed, but it was here for a reason...
        if (__HAL_TIM_GET_COUNTER(&htim2) >= FREQUENCY)
        {
            // __HAL_TIM_SetCounter(&htim2, FREQUENCY - incrementAmount); 
        }

        __HAL_TIM_SetAutoreload(&htim2, FREQUENCY);
    }
}

void ok_clock_set_period() {
    int incrementAmount = encoderIsPressed ? 10 : 100;

    if (encoderDirection == 1)
    {
        ok_clock_set_frequency(FREQUENCY - incrementAmount);
    }
    else if (encoderDirection == -1)
    {
        ok_clock_set_frequency(FREQUENCY + incrementAmount);
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
 * @brief This function handles TIM2 global interrupt.
 */
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

/**
 * @brief This function handles TIM1 capture compare interrupt.
 */
void TIM1_CC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim1);
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
}

/**
 * @brief Input Capture Callback for all TIMx configured in Input Capture mode
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1) {
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