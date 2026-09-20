/*******************************************************************************
* File Name:   thread_led.c
*
* Description: LED blinky thread implementations for the ThreadX RTOS demo.
*
*              Two threads cooperate via a semaphore to blink the USER LED:
*
*              thread_main_task_entry()
*                  - Sleeps for USER_LED_TOGGLE_PERIOD_MS (1 s), then
*                    releases (puts) led_semaphore.
*
*              thread_blinky_entry()
*                  - Waits (gets) led_semaphore indefinitely, then
*                    toggles the USER LED GPIO pin.
*
*              Global ThreadX objects and stack buffers for both threads
*              are statically allocated in this file and declared extern
*              in app_threadx.h.
*
********************************************************************************
* (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

#include <stdio.h>
#include "app_threadx.h"


/*******************************************************************************
* Global Variables - Thread Stacks and RTOS Objects
*******************************************************************************/
/* Stack for the Blinky thread - 8-byte aligned as required by Cortex-M ABI */
UCHAR thread_blinky_stack[THREAD_STACK_SIZE] __attribute__((aligned(8)));

/* Stack for the Main Task thread - 8-byte aligned */
UCHAR thread_main_task_stack[THREAD_STACK_SIZE] __attribute__((aligned(8)));

/* Static backing buffer for the ThreadX byte pool (used by App_ThreadX_Init) */
UCHAR byte_pool_buffer[TX_APP_MEM_POOL_SIZE] __attribute__((aligned(8)));

/* ThreadX thread control blocks */
TX_THREAD    thread_blinky;      /* Blinky thread - toggles USER LED on semaphore signal  */
TX_THREAD    thread_main_task;   /* Main task thread - generates periodic semaphore signal */

/* ThreadX kernel objects shared between threads */
TX_BYTE_POOL my_byte_pool;       /* Byte pool for optional dynamic allocation              */
TX_SEMAPHORE led_semaphore;      /* Binary-style semaphore: main_task -> blinky signalling */


/*******************************************************************************
* Function Name: thread_blinky_entry
********************************************************************************
* Summary:
*   Blinky thread - waits on led_semaphore and toggles the USER LED.
*
*   Execution flow (runs forever):
*     1. Call tx_semaphore_get() with TX_WAIT_FOREVER.
*        The thread is suspended here until thread_main_task_entry() calls
*        tx_semaphore_put().
*     2. When the semaphore is obtained, invert the USER LED GPIO pin state.
*     3. Loop back to step 1.
*
*   This pattern decouples timing (handled by thread_main_task_entry) from
*   the GPIO toggle action, demonstrating semaphore-based thread signalling.
*
* Parameters:
*  ULONG input : Thread creation parameter - not used, suppressed with macro.
*
* Return:
*  VOID  (ThreadX tasks must not return)
*
*******************************************************************************/
VOID thread_blinky_entry(ULONG input)
{
    /* Suppress unused-parameter compiler warning */
    TX_PARAMETER_NOT_USED(input);

    for(;;)
    {
        /* Block indefinitely until the Main Task thread releases the semaphore.
         * TX_WAIT_FOREVER means no timeout - the thread will wait as long as needed. */
        tx_semaphore_get(&led_semaphore, TX_WAIT_FOREVER);

        /* Semaphore obtained: invert the USER LED output register bit.
         * Cy_GPIO_Inv() performs a read-modify-write on the GPIO DATA register. */
        Cy_GPIO_Inv(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);
    }
}


/*******************************************************************************
* Function Name: thread_main_task_entry
********************************************************************************
* Summary:
*   Main Task thread - generates a periodic semaphore signal every
*   USER_LED_TOGGLE_PERIOD_MS milliseconds to drive the Blinky thread.
*
*   Execution flow (runs forever):
*     1. Call tx_thread_sleep() to block for USER_LED_TOGGLE_PERIOD_MS ticks.
*        This suspends the thread (no CPU wasted busy-waiting) and allows
*        lower-priority or equal-priority threads to run.
*     2. Wake up after the sleep interval expires.
*     3. Call tx_semaphore_put() to increment led_semaphore and unblock the
*        Blinky thread so it can toggle the LED.
*     4. Loop back to step 1.
*
* Parameters:
*  ULONG input : Thread creation parameter - not used, suppressed with macro.
*
* Return:
*  VOID  (ThreadX tasks must not return)
*
*******************************************************************************/
VOID thread_main_task_entry(ULONG input)
{
    /* Suppress unused-parameter compiler warning */
    TX_PARAMETER_NOT_USED(input);

    for(;;)
    {
        /* Sleep for USER_LED_TOGGLE_PERIOD_MS RTOS ticks (1000 ms = 1 s).
         * The thread is suspended for this duration; no CPU cycles are wasted. */
        tx_thread_sleep(USER_LED_TOGGLE_PERIOD_MS);

        /* Release the semaphore to signal the Blinky thread to toggle the LED.
         * tx_semaphore_put() increments the semaphore count by 1, which will
         * unblock thread_blinky_entry() that is waiting on tx_semaphore_get(). */
        tx_semaphore_put(&led_semaphore);
    }
}
