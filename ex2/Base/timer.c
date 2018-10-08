#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"


void startTimer() {
    *TIMER1_CMD = 0x1;
}

void stopTimer() {
    *TIMER1_CMD = 0x2;
}

/*
 * function to setup the timer 
 */
void setupTimer(uint16_t period)
{
    *CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_TIMER1;
    *TIMER1_TOP = period; // period
    *TIMER1_IEN = 1; // enable timer interrupt generation
}
