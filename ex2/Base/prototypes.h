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
void setupNVIC();

/* 
 * GPIO
 */
void setupGPIO();
void handleButtons(uint32_t DIN);


#endif /* end of include guard: PROTOTYPES_header */