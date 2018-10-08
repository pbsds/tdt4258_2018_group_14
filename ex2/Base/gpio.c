#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"
#include "prototypes.h"
#include "songs.h"

/*
 * function to set up GPIO mode and interrupts
 */
void setupGPIO()
{
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_GPIO;	/* enable GPIO clock */

	// LEDs
	*GPIO_PA_CTRL = 2;	/* set high drive current */
	*GPIO_PA_MODEH = 0x55555555;	/* set pins A8-15 as output */
	*GPIO_PA_DOUT = 0xff00;	/* turn off LEDs D4-D8 (note: active low) */
	
	// Buttons
	*GPIO_PC_MODEL = 0x33333333; /* READ mode for 0-7 */
	*GPIO_PC_DOUT = 0x00ff; /* pull high */

}

void handleButtons(uint32_t DIN)
{
	DIN = ~DIN;

	// debug:
	*GPIO_PA_DOUT = ~DIN<<8;

	bool sw1 = (DIN & (0x1<<0)); // left
	bool sw2 = (DIN & (0x1<<1)); // up
	bool sw3 = (DIN & (0x1<<2)); // right
	bool sw4 = (DIN & (0x1<<3)); // down
	bool sw5 = (DIN & (0x1<<4)); // y
	bool sw6 = (DIN & (0x1<<5)); // x
	bool sw7 = (DIN & (0x1<<6)); // a
	bool sw8 = (DIN & (0x1<<7)); // b

	if (sw1) { // left
		setSong(DOGSONG);
	}
	if (sw2) { // up
		setSong(MT_PYRE);
	}
	if (sw3) { // right
		setSong(SPIDER_DANCE);
	}
	if (sw4) { // down
		setSong(CIRCUSGALOP);
	}
	if (sw5) { // y
		setWaveform(sawtooth);
	}
	if (sw6) { // x
		setWaveform(squarewave);
	}
	if (sw7) { // a
		startTimer();
	}
	if (sw8) { // b
		stopTimer();
	}
}
