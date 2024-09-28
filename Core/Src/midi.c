#include "midi.h"

#define MIDI_START_BYTE 0xFA
#define MIDI_STOP_BYTE 0xFC
#define MIDI_CONTINUE_BYTE 0xFB
#define MIDI_CLOCK_BYTE 0xF8
#define MIDI_SONG_POSITION_POINTER 0xF2
#define MIDI_RESET_BYTE 0xFF

int elapsed_time_index = 0;
volatile message_elapsed_time[24];

/**
 * @brief
 *
 * @param midi_buffer
 *
 * Start (0xFA): Indicates the start of a sequence.
 * Stop (0xFC): Indicates the end of a sequence.
 * Continue (0xFB): Resumes a paused sequence.
 * Timing Clock (0xF8): Sent 24 times per quarter note.
*/
void process_midi_message(uint8_t *midi_buffer)
{
    static uint32_t prev_time_stamp = 0;
    static uint32_t curr_time_stamp = 0;
    switch (midi_buffer[0])
    {
    case MIDI_START_BYTE:
        ok_clock_reset();
        ok_clock_start();
        break;
    
    case MIDI_STOP_BYTE:
        ok_clock_stop();
        break;
    
    case MIDI_CONTINUE_BYTE:
        ok_clock_reset(); // reset is specifically for ableton live in arrangment view. It doesn't send a START message, just a CONTINUE (unless starting from the beginning)
        ok_clock_start();
        break;
    
    case MIDI_SONG_POSITION_POINTER:
        break;

    case MIDI_CLOCK_BYTE:
        // get the elapsed time between the last clock pulse
        // prev_time_stamp = curr_time_stamp;
        // curr_time_stamp = __HAL_TIM_GetCounter(&htim2);

        // if (curr_time_stamp < prev_time_stamp)
        // {
        //     message_elapsed_time[elapsed_time_index] = curr_time_stamp + (0xFFFFFFFF - prev_time_stamp) + 1; // + 1 to account for the 0-based index
        // } else {
        //     message_elapsed_time[elapsed_time_index] = curr_time_stamp - prev_time_stamp;
        // }

        // elapsed_time_index++;
        // if (elapsed_time_index >= 24)
        // {
        //     elapsed_time_index = 0;
        // }

        ok_clock_advance();
        break;
    default:
        break;
    }
}