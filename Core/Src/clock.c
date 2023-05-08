#include "clock.h"

uint16_t overflow = 990;

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
}
void ok_clock_loop()
{
    // program code
    HAL_Delay(overflow);
    HAL_GPIO_WritePin(GPIOA, TRANSPORT_PPQN_1, 1);
    HAL_GPIO_WritePin(GPIOA, RESET_BTN_LED, 1);
    HAL_Delay(10);
    HAL_GPIO_WritePin(GPIOA, TRANSPORT_PPQN_1, 0);
    HAL_GPIO_WritePin(GPIOA, RESET_BTN_LED, 0);
}

void ok_clock_set_overflow() {
    int incrementAmount = 10;
    if (overflow > (incrementAmount + 100) && overflow < 6000)
    {
        if (encoderDirection == 1)
        {
            overflow -= 10;
        }
        else if (encoderDirection == -1)
        {
            overflow += 10;
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
            ok_clock_set_overflow();
        }
        else
        {
            encoderDirection = 1;
            ok_clock_set_overflow();
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
            // ok_clock_set_overflow();
        }
        else
        {
            encoderDirection = -1;
            // ok_clock_set_overflow();
        }
    }

    prevEncoderStateB = encoderStateB;
}