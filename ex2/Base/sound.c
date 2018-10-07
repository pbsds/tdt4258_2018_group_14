#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"
#include "prototypes.h"

#define SAMPLE_RATE 44100
#define AMPLITUDE 0x07f

/*
 * Functions to generate waveform samples
 * Counter is a value incremented at samplerate speed
 */
uint16_t sawtooth(uint16_t counter, uint32_t freq, uint16_t amplitude) {
    return (freq*counter*amplitude/SAMPLE_RATE) % amplitude;
}


/*
 * Used to play songs
 * songs are defined in songs.h
 */

static const uint32_t (*player_song)[2]; // set to something in songs.h

void set_song(const uint32_t song[][2]) {
    player_song = song;
}
bool step_song()
{ // call this each sample, returns true while playing
    return true;
}
 
