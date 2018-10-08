#ifndef PROTOTYPES_header
#define PROTOTYPES_header

#include <stdint.h>
#include <stdbool.h>

/*
 * helpers
 */

uint32_t random();


/*
 * Declaration of peripheral setup functions
 */
void setupDAC();
void disableDAC();
void setupNVIC();

/*
 * sound
 */
void setSong(const uint32_t(*)[2]);
void setWaveform(uint16_t (*wavepoint_f)(uint16_t, uint32_t, uint16_t));
bool stepSong();
uint32_t getNextSongTimestamp(const uint32_t song[][2], unsigned int index);
uint32_t getSongNote(const uint32_t song[][2], unsigned int index);

uint16_t sawtooth(uint16_t counter, uint32_t freq, uint16_t amplitude);
uint16_t squarewave(uint16_t counter, uint32_t freq, uint16_t amplitude);


/*
 * timer
 */
void setupTimer(uint32_t period);
void startTimer();
void stopTimer();


/*
 * GPIO
 */
void setupGPIO();
void handleButtons(uint32_t DIN);


#endif /* end of include guard: PROTOTYPES_header */
