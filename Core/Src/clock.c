#include "clock.h"

uint32_t TEMPO_CEILING = 160000;
uint32_t TEMPO_FLOOR = 6000;
uint32_t FREQUENCY = 45000;
uint16_t TEMPO_ADJUST = 500;

bool encoderIsPressed = false;

uint16_t PULSE = 0;
bool RUNNING = false;

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
    ok_clock_set_frequency(FREQUENCY);
    ok_clock_set_clock_source(HAL_GPIO_ReadPin(GPIOA, TOGGLE_SWITCH));
}

void ok_clock_start()
{
    RUNNING = true;
    switch (CLOCK_SOURCE)
    {
    case CLOCK_SOURCE_INTERNAL:
        // HAL_NVIC_DisableIRQ(TIM1_CC_IRQn); // not sure why this is here
        HAL_TIM_Base_Start_IT(&htim2);
        break;

    case CLOCK_SOURCE_EXTERNAL:
        HAL_TIM_IC_Stop_IT(&htim1, TIM_CHANNEL_3);
        break;

    case CLOCK_SOURCE_MIDI:
        // disable UART
        break;
    }
}

void ok_clock_stop()
{
    RUNNING = false;
    switch (CLOCK_SOURCE)
    {
    case CLOCK_SOURCE_INTERNAL:
        HAL_NVIC_DisableIRQ(TIM1_CC_IRQn); // not sure why this is here
        HAL_TIM_Base_Stop_IT(&htim2);
        break;
    
    case CLOCK_SOURCE_EXTERNAL:
        HAL_TIM_IC_Stop_IT(&htim1, TIM_CHANNEL_3);
        break;
    
    case CLOCK_SOURCE_MIDI:
        // disable UART
        break;
    }
}

void ok_clock_advance()
{
    if (RUNNING == false)
        return;

    HAL_GPIO_WritePin(GPIOA, TRANSPORT_PPQN, HIGH);
    HAL_TIM_Base_Start_IT(&htim6); // start TIM6 to trigger TRANSPORT_PPQN pin low after short delay

    if (PULSE == 0)
    {
        HAL_GPIO_WritePin(GPIOA, CLOCK_OUTPUT, LOW); // gate outs are inverted
        HAL_GPIO_WritePin(GPIOA, TRANSPORT_QUARTER_NOTE, HIGH);
        HAL_GPIO_WritePin(GPIOA, RESET_BTN_LED, HIGH);
    }

    if (PULSE == 2)
    {
        HAL_GPIO_WritePin(GPIOA, CLOCK_RESET_OUTPUT, HIGH); // inverted || always setting this high just incase a reset was triggered
        HAL_GPIO_WritePin(GPIOA, TRANSPORT_RESET, LOW);
        HAL_GPIO_WritePin(GPIOA, CLOCK_OUTPUT, HIGH); // inverted
        HAL_GPIO_WritePin(GPIOA, TRANSPORT_QUARTER_NOTE, LOW);
        HAL_GPIO_WritePin(GPIOA, RESET_BTN_LED, LOW);
    }

    // FOR JUSTIN ***************
    // if (PULSE == 96) {
    //     HAL_GPIO_WritePin(GPIOA, CLOCK_OUTPUT, LOW); // gate outs are inverted
    // }

    // if (PULSE == 108) {
    //     HAL_GPIO_WritePin(GPIOA, CLOCK_OUTPUT, HIGH); // inverted
    // }
    // FOR JUSTIN ***************

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

    __HAL_TIM_SetCounter(&htim1, 0); // reset after each input capture
    __HAL_TIM_SetCounter(&htim2, 0); // reset after each input capture
    __HAL_TIM_ENABLE(&htim2);        // re-enable TIM2 (it gets disabled should the pulse count overtake PPQN before a new input capture event occurs)
    uint32_t inputCapture = __HAL_TIM_GetCompare(&htim1, TIM_CHANNEL_3);
    ok_clock_set_frequency(inputCapture / PPQN);
    PULSE = 0;
    ok_clock_advance();
}

void ok_clock_reset()
{
    PULSE = 0;
    HAL_GPIO_WritePin(GPIOA, CLOCK_RESET_OUTPUT, LOW); // inverted
    HAL_GPIO_WritePin(GPIOA, TRANSPORT_RESET, HIGH);
}

void ok_clock_set_clock_source(int clock_source)
{
    
    HAL_TIM_Base_Stop_IT(&htim2);
    HAL_TIM_IC_Stop_IT(&htim1, TIM_CHANNEL_3);
    CLOCK_SOURCE = clock_source;
    ok_clock_reset();
    ok_clock_start();
}

/**
 * @brief Sets the tempo by setting the Timers overflow / reload value
 * 
 * @param frequency 
 */
void ok_clock_set_frequency(uint32_t frequency)
{
    if (frequency >= TEMPO_FLOOR && frequency <= TEMPO_CEILING)
    {
        FREQUENCY = frequency;

        // ensure counter reloads, might not be necessary
        if (__HAL_TIM_GET_COUNTER(&htim2) >= FREQUENCY)
        {
            __HAL_TIM_SetCounter(&htim2, FREQUENCY - 1);
        }

        __HAL_TIM_SetAutoreload(&htim2, FREQUENCY);
    }
}

// this function can be removed and its contents put in encoder interrupt function
void ok_clock_set_period() {
    int incrementAmount = encoderIsPressed ? 100 : TEMPO_ADJUST;

    if (encoderDirection == 1)
    {
        ok_clock_set_frequency(FREQUENCY - incrementAmount);
    }
    else if (encoderDirection == -1)
    {
        ok_clock_set_frequency(FREQUENCY + incrementAmount);
    }
}


void encoder_handle_rotation()
{
    int chanA = HAL_GPIO_ReadPin(GPIOA, ENC_CHAN_A);
    int chanB = HAL_GPIO_ReadPin(GPIOA, ENC_CHAN_B);

    if (chanA == 0 && chanB == 1)
    {
        encoderDirection = 1;
        ok_clock_set_period();
    }
    else if (chanA == 0 && chanB == 0)
    {
        encoderDirection = -1;
        ok_clock_set_period();
    }
}