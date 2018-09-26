        .syntax unified

        .include "efm32gg.s"

  /////////////////////////////////////////////////////////////////////////////
  //
  // Exception vector table
  // This table contains addresses for all exception handlers
  //
  /////////////////////////////////////////////////////////////////////////////

        .section .vectors

        .long   stack_top               /* Top of Stack                 */
        .long   _reset                  /* Reset Handler                */
        .long   dummy_handler           /* NMI Handler                  */
        .long   dummy_handler           /* Hard Fault Handler           */
        .long   dummy_handler           /* MPU Fault Handler            */
        .long   dummy_handler           /* Bus Fault Handler            */
        .long   dummy_handler           /* Usage Fault Handler          */
        .long   dummy_handler           /* Reserved                     */
        .long   dummy_handler           /* Reserved                     */
        .long   dummy_handler           /* Reserved                     */
        .long   dummy_handler           /* Reserved                     */
        .long   dummy_handler           /* SVCall Handler               */
        .long   dummy_handler           /* Debug Monitor Handler        */
        .long   dummy_handler           /* Reserved                     */
        .long   dummy_handler           /* PendSV Handler               */
        .long   dummy_handler           /* SysTick Handler              */

        /* External Interrupts */
        .long   dummy_handler
        .long   main                    /* GPIO even handler */
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   main                    /* GPIO odd handler */
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler

        .section .text

  /////////////////////////////////////////////////////////////////////////////
  //
  // Reset handler
  // The CPU will start executing here after a reset
  //
  /////////////////////////////////////////////////////////////////////////////

        .globl  _reset
        .type   _reset, %function
        .thumb_func
_reset:
    // setup GPIO clock
    ldr r0, =CMU_BASE
    ldr r1, [r0, CMU_HFPERCLKEN0]       //- load GPIO clock enable

    mov r2, #1
    lsl r2, r2, CMU_HFPERCLKEN0_GPIO    //- find the bit which enables the GPIO clock
    orr r1, r1, r2                      //- set that bit

    str r1, [r0, CMU_HFPERCLKEN0]       //- write that change to MMIO register

    // setup LEDS GPIO as writeable
    ldr r0, =GPIO_PA_BASE               //- set GPIO_CTRL write mode
    mov r2, #0x2
    str r2, [r0, GPIO_CTRL]
	
    ldr r1, = 0x55555555                //- enable LEDs 8-15
    str r1, [r0, GPIO_MODEH]

    // setup button GPIO as readable
    ldr r0, =GPIO_PC_BASE               //- set GPIO_CTRL READ mode
    ldr r1, =0x33333333                 //- enable BUTTONS 0-7 by writing HIGH to them
    str r1, [r0, GPIO_MODEL]

    ldr r1, =0xff                       //- enable pull-up on buttons
    str r1, [r0, GPIO_DOUT]

    // setup GPIO interrupt pins
    ldr r0, =GPIO_BASE
    ldr r1, =0x22222222
    str r1, [r0, GPIO_EXTIPSELL]        //- set port cs pins 0-7 as interrupt generators

//    ldr r1, =0xff
//    str r1, [r0, GPIO_EXTIRISE]           //- generate interrupts on 0->1 transitions

    ldr r1, =0xff
    str r1, [r0, GPIO_EXTIFALL]         //- generate interrupts on 1->0 transitions

    ldr r1, =0xff
    str r1, [r0, GPIO_IEN]              //- enable interrupts on pins 0-7

    // Setup sleep
    ldr r0, =SCR
    ldr r1, =0x6                        //- Enable deep sleep, and go into deep sleep when returning from interrupt
    str r1, [r0]

    // Disable SRAM
    ldr r0, =EMU_BASE
	mov r1, #0x4                        //- Disable SRAM blocks 1-3
	//str r1, [r0, #0x004]                //- Write to EMU_MEMCTRL (doesn't work)

    // lower clock frequency
    ldr r0, =CMU_BASE
    ldr r1, [r0, #0x004]                //- get current frequency divide factor of CMU_HFCORECLKDIV
    ldr r2, [r0, CMU_HFPERCLKDIV]       //- get current frequency divide factor of CMU_HFPERCLKDIV
    mov r3, #100                        //- make it hundred times higher!
    mul r1, r1, r3
    mul r2, r2, r3
    str r1, [r0, #0x004]                //- store
    str r1, [r0, CMU_HFPERCLKDIV]       //- store

    // set initial state
    mov r7, #0b00000010                 //- the leds to show
    mov r10, #0                         //- do_invert = False
    ldr r0, =GPIO_PA_BASE               //- Load in entry point for LEDs
    lsl r1, r7, 8                       //- Right shift the current light position
    str r1, [r0, GPIO_DOUT]

    // clear interrupt flags
    ldr r0, =GPIO_BASE
    ldr r1, [r0, GPIO_IF]
    str r1, [r0, GPIO_IFC]

    // enable interrupt handling
    ldr r0, =ISER0
    ldr r1, =0x802
    str r1, [r0]	
    
    wfi
    b . //- safety for GDB


// The main loop iteration
	.thumb_func
main:

    // read interrupt states
    ldr r0, =GPIO_BASE
    ldr r8, [r0, GPIO_IF]               //- read button states
    str r8, [r0, GPIO_IFC]              //- clear GPIO interrupt

    push {LR}
	
    //mvn r8, r8

switch:

    //do shifting
    and r1, r8, #0x1                    //- check if button SW1 is pressed
    cbnz r1, main_2                     //- skip shiftleft if not pressed
    bl shiftright
main_2:
    and r1, r8, #0x4                    //- check if button SW3 is pressed
    cbnz r1, main_3                     //- skip shiftright if not pressed
    bl shiftleft
main_3:
    and r1, r8, #0x2                    //- check if button SW2 is pressed
    cbnz r1, main_4                     //- skip light if not pressed
    bl light
main_4:
    and r1, r8, #0x8                    //- check if button SW4 is pressed
    cbnz r1, main_5                     //- skip lightnegate if not pressed
    bl lightnegate
main_5:
    ldr r0, =GPIO_PA_BASE               //- Load in entry point for LEDs
    lsl r1, r7, 8                       //- Right shift the current light position

    cmp r10, #1                         //- Check if you should negate the light
    beq main_6                          //- Skip if you are not supposed to negate
    mvn r1, r1                          //- Negate the bits, inverting the light
main_6:
    str r1, [r0, GPIO_DOUT]             //- Set the LEDs to the right state

    pop {r0}
    bx r0
    wfi //- safety


////////////////////////////////////////////////////////////////////////////
//
//  Functions
//
////////////////////////////////////////////////////////////////////////////


shiftleft:
    push {LR}
    cmp r7, #0x01                       //- Check if light already all to the left
    beq shiftleft_end                   //- Skip if light cant be moved any more to the left
    lsr r7, r7, 1                       //- Move light to the left
shiftleft_end:
    pop {r0}                            //- return
    bx r0

    .thumb_func
shiftright:
    push {LR}
    cmp r7, #0x80                       //- Check if light already all to the right
    beq shiftright_end                  //- Skip if light cant be moved any more to the right
    lsl r7, r7, 1                       //- Move light to the right
shiftright_end:
    pop {r0}                            //- return
    bx r0

    .thumb_func
light:
    push {LR}
    mov r10, #1                         //- Set light to be positive
    pop {r0}                            //- return
    bx r0


    .thumb_func
lightnegate:
    push {LR}
    mov r10, #0                         //- Set light to be negated
    pop {r0}                            //- return
    bx r0


  /////////////////////////////////////////////////////////////////////////////
  //
  // GPIO handler
  // The CPU will jump here when there is a GPIO interrupt
  //
  /////////////////////////////////////////////////////////////////////////////

    .thumb_func
gpio_handler:
    ldr r0, =GPIO_BASE
    ldr r1, [r0, GPIO_IF]
    str r1, [r0, GPIO_IFC]
    bx lr

  /////////////////////////////////////////////////////////////////////////////
        .thumb_func
dead:
    wfi
    // Debug
    //ldr r0, =GPIO_BASE
    //ldr r1, [r0, GPIO_IF]
    //ldr r0, =ISER0
    //ldr r1, [r0]
    //ldr r0, =ISPR0
    //ldr r1, [r0]
    //ldr r0, =IABR0
    //ldr r1, [r0]
    // Debug End
    b dead

        .thumb_func
dummy_handler:
        b .                             //- do nothing
