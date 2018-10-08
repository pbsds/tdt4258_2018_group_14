#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"
#include "songs.h"
#include "prototypes.h"

/*
 * The period between sound samples, in clock cycles
 */
#define   SAMPLE_PERIOD   317 // 14Mhz/317 = ~44.1khz

int main(void)
{

	/*
	 * Call the peripheral setup functions
	 */
	setupGPIO();
	setupDAC();
	setupTimer(SAMPLE_PERIOD);

	/*
	 * Enable interrupt handling
	 */
	setupNVIC();

	//initial state
	setSong(DOGSONG);
	startTimer();

	__asm__("wfi");
	while (1); // GBD safety
	return 0;
}

void setupNVIC()
{
	*SCR            |= 0x2;        // Enable SLEEP ON EXIT
	//*SCR            |= 0x4;        // Enable DEEPSLEEP
	// ^ DEEPSLEEP disables the SysTick timer, which cuases TIMER1 to not work

	// general improvements:
	//*CMU_HFPERCLKDIV *= 100;      // increase high frequency clock divide factor
	*EMU_MEMCTL     = 0x3;        // Disable SRAM blocks 1 and 2, but not 3

	// enable interrupts generators
	*TIMER1_IEN     = 1;          // enable TIMER1 interrupt generation
	*GPIO_EXTIPSELL = 0x22222222; // set port C pin 0-7 as interrupt generators
	//*GPIO_EXTIRISE  = 0xff;       // generate interrupts on 0->1 transitions
	*GPIO_EXTIFALL  = 0xff;       // generate interrupts on 1->0 transitions
	*GPIO_IEN       = 0xff;       // enable interrupt generation on port 0-7

	// enable interrupt handlers:
	*ISER0 |= 0x1<<12; // TIMER1
	*ISER0 |= 0x1<<11; // GPIO_ODD
	*ISER0 |= 0x1<< 1; // GPIO_EVEN

	// clear interrupt flags
	*TIMER1_IFC = 1;
	*GPIO_IFC = *GPIO_IF;
}
