/*******************************************************************************
* File Name:   app_threadx.c
*
* Description: ThreadX application initialization.
*              Creates the byte-pool, synchronization semaphore, and all
*              application threads required for the LED-blinky demonstration.
*              Also provides the mandatory tx_application_define() hook that
*              the ThreadX kernel calls during start-up.
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

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "app_threadx.h"


/*******************************************************************************
* Function Name: App_ThreadX_Init
********************************************************************************
* Summary:
*   Creates all ThreadX kernel objects needed by the application.
*
*   Object creation order:
*     1. Byte pool  - provides heap memory for future dynamic allocations.
*     2. Semaphore  - used for thread-to-thread signalling (main task -> blinky).
*     3. Blinky thread    - waits on semaphore, then toggles the USER LED.
*     4. Main task thread - delays for USER_LED_TOGGLE_PERIOD_MS then signals.
*     5. Main thread      - performs hardware init at boot (highest priority),
*                           then drops to background priority.
*
*   Scenario - Simple LED Blinky with Semaphore:
*   =============================================
*   - Main Task Thread: sleep USER_LED_TOGGLE_PERIOD_MS -> tx_semaphore_put()
*   - Blinky Thread:    tx_semaphore_get() -> toggle USER LED
*
*   Demonstrates basic RTOS primitives:
*     - Semaphore   : thread-to-thread signalling without shared state
*     - Thread Sleep: periodic, blocking delay
*     - GPIO Control: USER LED output toggle
*
* Parameters:
*  VOID
*
* Return:
*  UINT  TX_SUCCESS if all objects are created successfully,
*        TX_POOL_ERROR / TX_THREAD_ERROR on any failure.
*
*******************************************************************************/
UINT App_ThreadX_Init(VOID)
{
    UINT ret = TX_SUCCESS;

    /* -------------------------------------------------------------------
     * 1. Create a byte pool for dynamic memory allocation.
     *    The pool is backed by the statically allocated byte_pool_buffer.
     * ------------------------------------------------------------------- */
    if (tx_byte_pool_create(&my_byte_pool, TX_NAME_BYTE_POOL, byte_pool_buffer, TX_APP_MEM_POOL_SIZE) != TX_SUCCESS)
    {
        return TX_POOL_ERROR;  /* Fatal: failed to create memory pool */
    }

    /* -------------------------------------------------------------------
     * 2. Create the LED semaphore with an initial count of 0.
     *    The blinky thread will block on this semaphore until the main
     *    task thread releases it after each timing interval.
     * ------------------------------------------------------------------- */
    tx_semaphore_create(&led_semaphore, TX_NAME_SEMAPHORE, 0 /* initial count = 0 (blocking) */);

    /* -------------------------------------------------------------------
     * 3. Create the Blinky thread.
     *    Entry  : thread_blinky_entry (defined in thread_led.c)
     *    Action : blocks on led_semaphore, then toggles USER LED
     * ------------------------------------------------------------------- */
    if (tx_thread_create(&thread_blinky, TX_NAME_THREAD_BLINKY, thread_blinky_entry, 0,
                         thread_blinky_stack, THREAD_STACK_SIZE,
                         THREAD_PRIORITY_BLINKY, THREAD_PRIORITY_BLINKY,
                         TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
    {
        return TX_THREAD_ERROR;  /* Fatal: failed to create blinky thread */
    }

    /* -------------------------------------------------------------------
     * 4. Create the Main Task thread.
     *    Entry  : thread_main_task_entry (defined in thread_led.c)
     *    Action : sleeps for USER_LED_TOGGLE_PERIOD_MS then signals semaphore
     * ------------------------------------------------------------------- */
    if (tx_thread_create(&thread_main_task, TX_NAME_THREAD_MAIN_TASK, thread_main_task_entry, 0,
                         thread_main_task_stack, THREAD_STACK_SIZE,
                         THREAD_PRIORITY_MAIN_TASK, THREAD_PRIORITY_MAIN_TASK,
                         TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
    {
        return TX_THREAD_ERROR;  /* Fatal: failed to create main task thread */
    }

    /* -------------------------------------------------------------------
     * 5. Create the Main (boot) thread.
     *    Entry    : thread_main_entry (defined in thread_main.c)
     *    Action   : initializes UART, boots PPCA cores, then lowers own
     *               priority to THREAD_PRIORITY_MAIN_BACKGROUND
     *    Priority : THREAD_PRIORITY_MAIN_BOOT (highest) at creation.
     * ------------------------------------------------------------------- */
    if (tx_thread_create(&thread_main, TX_NAME_THREAD_MAIN, thread_main_entry, 0,
                         thread_main_stack, THREAD_STACK_SIZE_MAIN,
                         THREAD_PRIORITY_MAIN_BOOT, THREAD_PRIORITY_MAIN_BOOT,
                         TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
    {
        return TX_THREAD_ERROR;  /* Fatal: failed to create main boot thread */
    }

    return ret;  /* TX_SUCCESS */
}


/*******************************************************************************
* Function Name: tx_application_define
********************************************************************************
* Summary:
*  Mandatory ThreadX callback invoked by the kernel during tx_kernel_enter().
*  Responsible for creating all application RTOS objects before the scheduler
*  starts. This implementation delegates to App_ThreadX_Init().
*
*  Note: "first_unused_memory" pointer (supplied by the kernel's low-level
*  init) is not used here because a statically allocated byte pool is used
*  instead. TX_PARAMETER_NOT_USED() suppresses compiler warnings.
*
* Parameters:
*  VOID *first_unused_memory - pointer to the first unused RAM address
*                              (provided by the ThreadX port, not used here)
*
* Return:
*  VOID
*
*******************************************************************************/
VOID tx_application_define(VOID *first_unused_memory)
{
    /* Suppress unused-parameter warning; static pool is used instead */
    TX_PARAMETER_NOT_USED(first_unused_memory);

    /* Optionally override NVIC priorities when using BASEPRI-based ISR
     * management (TX_PORT_USE_BASEPRI). Uncomment if needed:
     *
     * NVIC_SetPriority(PendSV_IRQn,  7);
     * NVIC_SetPriority(SysTick_IRQn, 6);
     * NVIC_SetPriority(SVCall_IRQn,  1);
     */
// #if defined (TX_PORT_USE_BASEPRI)
//     NVIC_SetPriority(PendSV_IRQn, 7);
//     NVIC_SetPriority(SysTick_IRQn, 6);
//     NVIC_SetPriority(SVCall_IRQn, 1);
// #endif

    /* Create all RTOS objects; halt if any creation fails */
    if (App_ThreadX_Init() != TX_SUCCESS)
    {
        CY_ASSERT(0);  /* Fatal: RTOS object creation failed */
    }
}


/* ---------------------------------------------------------------------------
 * IAR-specific weak stubs for ThreadX TrustZone secure-stack functions.
 * These are provided so the project compiles cleanly in secure-only mode
 * (TX_SINGLE_MODE_SECURE) where TrustZone stack switching is not required.
 * The #pragma weak declarations ensure the real implementations (if linked)
 * take precedence over these stubs.
 * --------------------------------------------------------------------------- */
#if defined(__ICCARM__) || defined(__ARMCC_VERSION)

/* Stub: secure-mode stack allocation - feature not enabled in this config */
#pragma weak _tx_thread_secure_mode_stack_allocate
UINT _tx_thread_secure_mode_stack_allocate(TX_THREAD *thread_ptr, ULONG stack_size)
{
    (void)thread_ptr;
    (void)stack_size;
    return TX_FEATURE_NOT_ENABLED;
}

/* Stub: secure-mode stack free - feature not enabled in this config */
#pragma weak _tx_thread_secure_mode_stack_free
UINT _tx_thread_secure_mode_stack_free(TX_THREAD *thread_ptr)
{
    (void)thread_ptr;
    return TX_FEATURE_NOT_ENABLED;
}

/* Stub: secure-mode stack initialise - no-op in secure-only mode */
#pragma weak _tx_thread_secure_mode_stack_initialize
void _tx_thread_secure_mode_stack_initialize(void)
{
    /* Empty - no TrustZone context switching needed in secure-only mode */
}

/* Stub: save secure stack context - returns success without action */
#pragma weak _tx_thread_secure_stack_context_save
UINT _tx_thread_secure_stack_context_save(TX_THREAD *thread_ptr)
{
    (void)thread_ptr;
    return TX_SUCCESS;
}

/* Stub: restore secure stack context - returns success without action */
#pragma weak _tx_thread_secure_stack_context_restore
UINT _tx_thread_secure_stack_context_restore(TX_THREAD *thread_ptr)
{
    (void)thread_ptr;
    return TX_SUCCESS;
}

#endif /* __ICCARM__ || __ARMCC_VERSION */