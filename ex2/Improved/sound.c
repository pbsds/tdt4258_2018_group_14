#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"
#include "prototypes.h"

#define SAMPLE_RATE 44100
#define AMPLITUDE 0x07f

/*
 * Functions to generate waveform samples
 * Counter is a value incremented at samplerate speed
 * These are currently the main computation draw, due to the divide
 * A possible solution might be a lookup table or something
 */
uint16_t sawtooth(uint16_t counter, uint32_t freq, uint16_t amplitude)
{
    return (freq*counter*amplitude/SAMPLE_RATE) % amplitude;
}
uint16_t squarewave(uint16_t counter, uint32_t freq, uint16_t amplitude)
{
    return ((freq*counter*2/SAMPLE_RATE)%2) ? amplitude : 0;
}


/*
 * Used to play songs
 * songs are defined in songs.h
 */

static const uint32_t (*player_song)[2]; // set to something in songs.h
static uint32_t  player_mstime = 0; // milliseconds
static unsigned int  player_song_index = 0; // index into song
static uint16_t (*player_waveform)(uint16_t, uint32_t, uint16_t) = &sawtooth;
static uint32_t sample_counter = 0;
static uint32_t ms_counter = 0;

void setSong(const uint32_t song[][2])
{
    player_song = song;
    player_song_index = 0;
    player_mstime = 0;
    sample_counter = 0;
    ms_counter = 0;
}
void setWaveform(uint16_t (*wavepoint_f)(uint16_t, uint32_t, uint16_t))
{
    player_waveform = wavepoint_f;
}
// call this each sample, returns true while playing
bool stepSong()
{
    // advance time
    if (++ms_counter>=44) {
        player_mstime++;
        ms_counter = 0;
    }
    if (++sample_counter>=44100) { //maybe not reset this each second?
        sample_counter = 0;
    }


    uint32_t freq = getSongNote(player_song, player_song_index);

    // check if next note is to be played
    if (getNextSongTimestamp(player_song, player_song_index) <= player_mstime) {
        uint32_t next = getSongNote(player_song, player_song_index+1);

        if (freq == 0 && next == 0) return false;

        player_song_index++; //goto next note
        freq = next;
    }

    // play note
    if (freq != 0) {
        uint16_t sample = (*player_waveform)(sample_counter, freq, AMPLITUDE);
        *DAC0_CH1DATA = sample;
        *DAC0_CH0DATA = sample;
    }

    return true;
}


// helpers
uint32_t getNextSongTimestamp(const uint32_t song[][2], unsigned int index)
{
   return song[index][0];
   //return *(song+ index*2);
}
uint32_t getSongNote(const uint32_t song[][2], unsigned int index)
{
   return song[index][1];
   //return *(song+ index*2 + 1);
}
