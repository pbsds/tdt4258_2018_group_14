#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"
#include "prototypes.h"

/*
 * TIMER1 interrupt handler
 */
void __attribute__ ((interrupt)) TIMER1_IRQHandler()
{
    *TIMER1_IFC = 1;      // clear pending interrupt

    if (!stepSong())
        stopTimer();
}

/*
 * GPIO even pin interrupt handler
 */
void __attribute__ ((interrupt)) GPIO_EVEN_IRQHandler()
{
    uint32_t DIN = *GPIO_IF;
    *GPIO_IFC = DIN;  // clear pending interrupt
    *GPIO_PA_DOUT = 0x0000;
    handleButtons(~DIN);

}

/*
 * GPIO odd pin interrupt handler
 */
void __attribute__ ((interrupt)) GPIO_ODD_IRQHandler()
{
    uint32_t DIN = *GPIO_IF;
    *GPIO_IFC = DIN;  // clear pending interrupt
    *GPIO_PA_DOUT = 0x0000;
    handleButtons(~DIN);
}


/*
 * if other interrupt handlers are needed, use the following names:
 *
 * NMI_Handler
 * HardFault_Handler
 * MemManage_Handler
 * BusFault_Handler
 * UsageFault_Handler
 * Reserved7_Handler
 * Reserved8_Handler
 * Reserved9_Handler
 * Reserved10_Handler
 * SVC_Handler
 * DebugMon_Handler
 * Reserved13_Handler
 * PendSV_Handler
 * SysTick_Handler
 * DMA_IRQHandler
 * GPIO_EVEN_IRQHandler
 * TIMER0_IRQHandler
 * USART0_RX_IRQHandler
 * USART0_TX_IRQHandler
 * USB_IRQHandler
 * ACMP0_IRQHandler
 * ADC0_IRQHandler
 * DAC0_IRQHandler
 * I2C0_IRQHandler
 * I2C1_IRQHandler
 * GPIO_ODD_IRQHandler
 * TIMER1_IRQHandler
 * TIMER2_IRQHandler
 * TIMER3_IRQHandler
 * USART1_RX_IRQHandler
 * USART1_TX_IRQHandler
 * LESENSE_IRQHandler
 * USART2_RX_IRQHandler
 * USART2_TX_IRQHandler
 * UART0_RX_IRQHandler
 * UART0_TX_IRQHandler
 * UART1_RX_IRQHandler
 * UART1_TX_IRQHandler
 * LEUART0_IRQHandler
 * LEUART1_IRQHandler
 * LETIMER0_IRQHandler
 * PCNT0_IRQHandler
 * PCNT1_IRQHandler
 * PCNT2_IRQHandler
 * RTC_IRQHandler
 * BURTC_IRQHandler
 * CMU_IRQHandler
 * VCMP_IRQHandler
 * LCD_IRQHandler
 * MSC_IRQHandler
 * AES_IRQHandler
 * EBI_IRQHandler
 * EMU_IRQHandler
 *
 */
