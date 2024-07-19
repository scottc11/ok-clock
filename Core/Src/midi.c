#include "midi.h"

#define MIDI_START_BYTE 0xFA
#define MIDI_STOP_BYTE 0xFC
#define MIDI_CONTINUE_BYTE 0xFB
#define MIDI_CLOCK_BYTE 0xF8

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
    case MIDI_START_BYTE:
        ok_clock_reset();
        break;
    case MIDI_STOP_BYTE:
        ok_clock_reset();
        break;
    case MIDI_CONTINUE_BYTE:
        ok_clock_reset();
        break;
    case MIDI_CLOCK_BYTE:
        ok_clock_advance();
        break;
    default:
        break;
    }
}