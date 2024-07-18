#include "midi.h"

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
    switch (midi_buffer[0])
    {
    case 0xFA:
        ok_clock_reset();
        break;
    case 0xFC:
        ok_clock_reset();
        break;
    case 0xFB:
        ok_clock_reset();
        break;
    case 0xF8:
        ok_clock_advance();
        break;
    default:
        break;
    }
}