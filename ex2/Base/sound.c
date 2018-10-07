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
static uint32_t  player_mstime = 0; // milliseconds
static unsigned int  player_song_index = 0; // index into song
static uint32_t sample_counter = 0;
static uint32_t ms_counter = 0;

void set_song(const uint32_t song[][2]) {
    player_song = song;
    player_song_index = 0;
    player_mstime = 0;
    sample_counter = 0;
    ms_counter = 0;
}
bool step_song()
{ // call this each sample, returns true while playing
    // advance time
    if (++ms_counter>=44) {
        player_mstime++;
        ms_counter = 0;
    }
    if (++sample_counter>=44100) { //maybe not reset this each second?
        sample_counter = 0;
    }
    
    
    uint32_t freq = get_song_note(player_song, player_song_index);
    
    // check if next note is to be played
    if (get_next_song_timestamp(player_song, player_song_index) <= player_mstime) {
        uint32_t next = get_song_note(player_song, player_song_index+1);
        
        if (freq == 0 && next == 0) return false;
        
        player_song_index++; //goto next note
        freq = next;
    }
    
    // play note
    if (freq != 0) {
        uint16_t sample = sawtooth(sample_counter, freq, AMPLITUDE);
        *DAC0_CH1DATA = sample;
        *DAC0_CH0DATA = sample;
    }
    
    return true;
}
 

// helpers
uint32_t get_next_song_timestamp(const uint32_t song[][2], unsigned int index) {
   return song[index][0];
   //return *(song+ index*2);
}
uint32_t get_song_note(const uint32_t song[][2], unsigned int index) {
   return song[index][1];
   //return *(song+ index*2 + 1);
}
