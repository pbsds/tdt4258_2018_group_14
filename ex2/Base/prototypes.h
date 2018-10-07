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
void set_song(const uint32_t(*)[2]);
bool step_song();
uint32_t get_next_song_timestamp(const uint32_t song[][2], unsigned int index);
uint32_t get_song_note(const uint32_t song[][2], unsigned int index);


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
