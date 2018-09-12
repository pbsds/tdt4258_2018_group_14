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
        .long   gpio_handler            /* GPIO even handler */
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   dummy_handler
        .long   gpio_handler            /* GPIO odd handler */
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

    // set initial state
    mov r7, #0b00000010                 //- the leds to show
    mov r9, #0xff                       //- the previous button state
    mov r10, #0                         //- do invert



main:
    // read button states
    ldr r0, =GPIO_PC_BASE
    ldr r8, [r0, GPIO_DIN]              //- read button states

    mvn r1, r9
    mov r9, r8
    orr r8, r1, r8

    //do shifting
    and r1, r8, #0x1
    cbnz r1, main_2
    bl shiftleft
main_2:
    and r1, r8, #0x4
    cbnz r1, main_3
    bl shiftright
main_3:
    and r1, r8, #0x2
    cbnz r1, main_4
    bl light
main_4:
    and r1, r8, #0x8
    cbnz r1, main_5
    bl lightnegate
main_5:

    ldr r0, =GPIO_PA_BASE
    lsl r1, r7, 8

    cmp r10, #1
    beq main_6
    mvn r1, r1
main_6:
    str r1, [r0, GPIO_DOUT]



    b main                              //- loop

shiftleft:
    push {LR}
    cmp r7, #0x01
    beq shiftleft_end
    lsr r7, r7, 1
shiftleft_end:
    pop {LR}                            //- return
    mov PC, LR

    .thumb_func
shiftright:
    push {LR}
    cmp r7, #0x80
    beq shiftright_end
    lsl r7, r7, 1
shiftright_end:
    pop {LR}                            //- return
    mov PC, LR

    .thumb_func
light:
    push {LR}
    mov r10, #1
    pop {LR}                            //- return
    mov PC, LR


    .thumb_func
lightnegate:
    push {LR}
    mov r10, #0
    pop {LR}                            //- return
    mov PC, LR


data:
    .word 0                             //- led position

  /////////////////////////////////////////////////////////////////////////////
  //
  // GPIO handler
  // The CPU will jump here when there is a GPIO interrupt
  //
  /////////////////////////////////////////////////////////////////////////////

        .thumb_func
gpio_handler:

        b .  // do nothing

  /////////////////////////////////////////////////////////////////////////////

        .thumb_func
dummy_handler:
        b .  // do nothing
