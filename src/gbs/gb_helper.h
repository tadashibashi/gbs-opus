#ifndef GBS_OPUS_GB_HELPER_H
#define GBS_OPUS_GB_HELPER_H

#include <cmath>
#include "audio/gbs_player.h"

const int MIDI_OFFSET = 21;
const int OCTAVE_NOTE_COUNT = 12;

double gbsq_to_midi(int gb_freq)
{
    double period = (2048.0 - (double)gb_freq) * 4.0;
    double freq   = 4194304.0 / period * 0.125;
    // equation from https://glassarmonica.com/science/frequency_midi.php
    return (OCTAVE_NOTE_COUNT/std::log(2.0) * std::log(freq/27.5) + MIDI_OFFSET);
}

double gbsq_to_freq(int gb_freq)
{
    double period = (2048.0 - (double)gb_freq) * 4.0;
    return 4194304.0 / period * 0.125;
}

double freq_to_midi(double freq)
{
    return (OCTAVE_NOTE_COUNT/std::log(2.0) * std::log(freq/27.5) + MIDI_OFFSET);
}

double gbwv_to_midi(int gb_freq)
{
    double period = (2048.0 - (double)gb_freq) * 2.0;
    double freq   = 4194304.0 / period * 0.03125;
    return (OCTAVE_NOTE_COUNT/std::log(2.0) * std::log(freq/27.5) + MIDI_OFFSET);
}

double getnote(long div)
{
    double n = 0;

    if (div>0) {
        n = NOTE(div);
    }

    if (n < 0) {
        n = 0;
    } else if (n >= MAXOCTAVE*12) {
        n = MAXOCTAVE-1;
    }

    return n;
}

#endif //GBS_OPUS_GB_HELPER_H
