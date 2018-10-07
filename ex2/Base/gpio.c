#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

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
	*GPIO_PA_DOUT = DIN<<8;
}
