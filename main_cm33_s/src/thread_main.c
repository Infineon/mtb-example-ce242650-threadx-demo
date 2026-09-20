/*******************************************************************************
* File Name:   thread_main.c
*
* Description: Main boot thread for the ThreadX RTOS blinky application.
*
*              thread_main_entry() runs at the HIGHEST priority
*              (THREAD_PRIORITY_MAIN_BOOT) immediately after the ThreadX
*              scheduler starts, ensuring hardware initialization completes
*              before any application thread executes.
*
*              Responsibilities:
*                1. Initialize the debug UART (SCB-based) and retarget-io.
*                2. Configure and enable the PPCA peripheral (EPU blocks).
*                3. Boot PPCA CM33 Core 0 from CORE0_IMAGE_ADDRESS in flash.
*                4. Boot PPCA CM33 Core 1 from CORE1_IMAGE_ADDRESS in flash.
*                5. Lower its own thread priority to THREAD_PRIORITY_MAIN_BACKGROUND
*                   so LED-blinky threads can run at full speed.
*                6. Sleep indefinitely as a background idle task.
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
#include "cy_retarget_io.h"

#include "app_threadx.h"

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Stack for the Main (boot) thread - larger than task stacks because it performs
 * UART init, printf calls, and system boot operations that require more stack depth */
UCHAR     thread_main_stack[THREAD_STACK_SIZE_MAIN] __attribute__((aligned(8)));

/* ThreadX control block for the Main boot thread */
TX_THREAD thread_main;


/* Debug UART driver objects ---------------------------------------------------
 * DEBUG_UART_context  : PDL SCB UART context (internal driver state)
 * DEBUG_UART_hal_obj  : HAL UART handle, passed to cy_retarget_io_init()
 * Both are static - not accessed from other files.
 * ---------------------------------------------------------------------------- */
static cy_stc_scb_uart_context_t    DEBUG_UART_context; /* SCB UART PDL context     */
static mtb_hal_uart_t               DEBUG_UART_hal_obj; /* HAL UART object for printf */


/*******************************************************************************
* Function Name: thread_main_entry
********************************************************************************
* Summary:
*  Main boot/initialization thread entry function.
*
*  Execution flow:
*    1. Init SCB UART (DEBUG_UART_HW) and enable it.
*    2. Set up the HAL UART wrapper (mtb_hal_uart_setup).
*    3. Initialize retarget-io so printf() outputs to the debug UART.
*    4. Print application banner and boot status messages.
*    5. Define flash addresses for PPCA core images.
*    6. Initialize and enable the PPCA configuration block.
*    7. Configure EPU (Event Processing Unit) PU_T2 and Combiner blocks
*       for GPIO routing (GPIO1 and GPIO2 signals).
*    8. Load and start PPCA CM33 Core 0 from flash.
*    9. Load and start PPCA CM33 Core 1 from flash.
*   10. Lower own priority to THREAD_PRIORITY_MAIN_BACKGROUND.
*   11. Enter an infinite sleep loop (background idle; never wakes).
*
* Parameters:
*  ULONG input : Thread creation parameter - not used, suppressed with macro.
*
* Return:
*  VOID  (ThreadX tasks must not return)
*
*******************************************************************************/
VOID thread_main_entry(ULONG input)
{
    TX_PARAMETER_NOT_USED(input);   /* Suppress unused-parameter warning */

    cy_rslt_t result;

    /* -----------------------------------------------------------------------
     * Step 1 - Initialize the debug SCB UART (PDL layer).
     * DEBUG_UART_HW and DEBUG_UART_config are generated by Device Configurator
     * and placed in cycfg_peripherals.c / cycfg_peripherals.h.
     * ----------------------------------------------------------------------- */
    Cy_SCB_UART_Init(DEBUG_UART_HW, &DEBUG_UART_config, &DEBUG_UART_context);
    Cy_SCB_UART_Enable(DEBUG_UART_HW);  /* Enable the UART hardware block */

    /* -----------------------------------------------------------------------
     * Step 2 - Wrap the SCB UART with the HAL layer.
     * mtb_hal_uart_setup() bridges the PDL context to the HAL object used by
     * retarget-io. Halt on failure; debug output is required for all subsequent
     * status prints.
     * ----------------------------------------------------------------------- */
    result = mtb_hal_uart_setup(&DEBUG_UART_hal_obj, &DEBUG_UART_hal_config,
                                &DEBUG_UART_context, NULL);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);  /* Fatal: UART HAL setup failed */
    }

    /* -----------------------------------------------------------------------
     * Step 3 - Initialize retarget-io.
     * Routes standard printf()/scanf() via the HAL UART object so all
     * subsequent printf calls appear on the debug terminal.
     * ----------------------------------------------------------------------- */
    result = cy_retarget_io_init(&DEBUG_UART_hal_obj);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);  /* Fatal: retarget-io init failed */
    }

    /* -----------------------------------------------------------------------
     * Step 4 - Print application banner on the debug terminal.
     * "\x1b[2J\x1b[;H" is the ANSI escape sequence that clears the terminal
     * screen and moves the cursor to home position (row 0, col 0).
     * ----------------------------------------------------------------------- */
    printf("\x1b[2J\x1b[;H"); // ANSI ESC sequence for clear screen
    printf("************************************************************\r\n");
    printf("PSOC Control C3M/P8: ThreadX RTOS blinky\r\n");
    printf("************************************************************\r\n\n");
    printf("Initializing hardware...\r\n");

    /* -----------------------------------------------------------------------
     * Step 5 - Define flash memory addresses for PPCA core images.
     *
     * CORE0_IMAGE_ADDRESS / CORE1_IMAGE_ADDRESS:
     *   Start addresses of the PPCA CM33 Core 0 and Core 1 firmware images
     *   stored in secure flash, as defined by the linker memory map symbols.
     *
     * PPCA0_IMAGE_SIZE / PPCA1_IMAGE_SIZE:
     *   Usable code size of each PPCA image region, taken from the generated
     *   memory map so it tracks any linker layout change automatically.
     * ----------------------------------------------------------------------- */
    #define CORE0_IMAGE_ADDRESS   CYMEM_CM33_0_S_m33s_ppca0_nvm_C_S_START
    #define CORE1_IMAGE_ADDRESS   CYMEM_CM33_0_S_m33s_ppca1_nvm_C_S_START

    #define PPCA0_IMAGE_SIZE      CYMEM_CM33_0_S_ppca0_code_SIZE
    #define PPCA1_IMAGE_SIZE      CYMEM_CM33_0_S_ppca1_code_SIZE
    
    /* -----------------------------------------------------------------------
     * Step 6 - Initialize and enable the PPCA configuration block.
     * Cy_PPCA_CNFG_Init() writes the generated Device Configurator settings.
     * Cy_PPCA_Enable() activates the PPCA IP block so the CM33 sub-cores
     * can be loaded and started.
     * ----------------------------------------------------------------------- */
    printf("  - Configuring PPCA...\r\n");
    Cy_PPCA_CNFG_Init(PPCA_CNFG_HW, &PPCA_CNFG_config);
    Cy_PPCA_Enable(PPCA_CNFG_HW);   /* Enable PPCA after configuration */

    /* -----------------------------------------------------------------------
     * Step 7 - Configure the EPU (Event Processing Unit).
     *
     * The EPU routes GPIO signals through Programmable Unit Type-2 (PU_T2)
     * blocks into combiners, providing flexible I/O steering to PPCA cores.
     *
     *   PU_T2_2_GPIO1 -> COMBINER_IO1 : routes GPIO1 signal to PPCA
     *   PU_T2_3_GPIO2 -> COMBINER_IO2 : routes GPIO2 signal to PPCA
     * ----------------------------------------------------------------------- */
    printf("  - Configuring EPU...\r\n");
    Cy_PPCA_EPU_Enable(EPU_BLK_HW);  /* Power up EPU block */

    /* Configure GPIO1 PU_T2 and its associated signal combiner */
    Cy_PPCA_EPU_PU_T2_Configure(PU_T2_2_GPIO1_HW, PU_T2_2_GPIO1_INDEX, &PU_T2_2_GPIO1_put2_config);
    Cy_PPCA_EPU_PU_T2_Enable(PU_T2_2_GPIO1_HW, PU_T2_2_GPIO1_INDEX, PU_T2_2_GPIO1_ENABLE_MODE);
    Cy_PPCA_EPU_Combo_Configure(COMBINER_IO1_HW, COMBINER_IO1_INDEX, &COMBINER_IO1_combo_config);

    /* Configure GPIO2 PU_T2 and its associated signal combiner */
    Cy_PPCA_EPU_PU_T2_Configure(PU_T2_3_GPIO2_HW, PU_T2_3_GPIO2_INDEX, &PU_T2_3_GPIO2_put2_config);
    Cy_PPCA_EPU_PU_T2_Enable(PU_T2_3_GPIO2_HW, PU_T2_3_GPIO2_INDEX, PU_T2_3_GPIO2_ENABLE_MODE);
    Cy_PPCA_EPU_Combo_Configure(COMBINER_IO2_HW, COMBINER_IO2_INDEX, &COMBINER_IO2_combo_config);

    /* -----------------------------------------------------------------------
     * Step 8 - Boot PPCA CM33 Core 0.
     * Cy_System_Init_CPU0() copies the firmware image from flash into the
     * PPCA Core 0 TCM and releases the core from reset.
     * ----------------------------------------------------------------------- */
    printf("  - Booting PPCA CM33 Core 0...\r\n");
    Cy_System_Init_CPU0((void*)CORE0_IMAGE_ADDRESS, PPCA0_IMAGE_SIZE);

    /* -----------------------------------------------------------------------
     * Step 9 - Boot PPCA CM33 Core 1.
     * Same procedure as Core 0, using the Core 1 image address.
     * ----------------------------------------------------------------------- */
    printf("  - Booting PPCA CM33 Core 1...\r\n");
    Cy_System_Init_CPU1((void*)CORE1_IMAGE_ADDRESS, PPCA1_IMAGE_SIZE);

    /* Print final boot status messages */
    printf("\r\n");
    printf("Hardware initialization complete!\r\n");
    printf("PPCA CM33 Cores (0 & 1) are now running.\r\n");
    printf("\r\n");
    printf("Starting ThreadX RTOS Blinky demonstration...\r\n");
    printf("Watch the USER LED blink!\r\n");
    printf("\r\n");

    /* -----------------------------------------------------------------------
     * Step 10 - Demote own thread priority to background level.
     * Now that initialization is complete, lower priority to
     * THREAD_PRIORITY_MAIN_BACKGROUND so the Blinky and Main Task threads
     * (at priority 5) can preempt this thread freely.
     * TX_NULL for the old-priority output parameter means we discard the
     * previous value.
     * ----------------------------------------------------------------------- */
    tx_thread_priority_change(&thread_main, THREAD_PRIORITY_MAIN_BACKGROUND, TX_NULL);

    /* -----------------------------------------------------------------------
     * Step 11 - Background idle loop.
     * Sleep indefinitely. TX_WAIT_FOREVER suspends the thread permanently.
     * The thread is kept alive (not terminated) so its stack stays allocated
     * and it can serve as a watchdog hook in the future if needed.
     * ----------------------------------------------------------------------- */
    while(1)
    {
        /* Suspend thread forever - yields CPU to all other ready threads */
        tx_thread_sleep(TX_WAIT_FOREVER);
    }
}

