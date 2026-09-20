/***************************************************************************//**
* File Name:   main.c
*
* Description : Entry point for the CM33 Secure core.
*               Performs board-level hardware initialization, enables global
*               interrupts, then transfers control to the ThreadX RTOS kernel
*               via tx_kernel_enter(). The kernel will call
*               tx_application_define() to create all threads and RTOS objects.
*
* Related Document : See README.md
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


/******************************************************************************
 * Header Files
 *****************************************************************************/
#include "cy_pdl.h"          /* PDL (Peripheral Driver Library) */
#include "cybsp.h"           /* Board support package - BSP pin and peripheral init */
#include "cy_retarget_io.h"  /* Retarget printf/scanf to UART */
/* ThreadX API - provides tx_kernel_enter and RTOS definitions */
#include "tx_api.h"


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  System entry point for the CM33 Secure core.
*  1. Initializes all board peripherals via cybsp_init().
*  2. Enables CPU interrupts.
*  3. Starts the ThreadX RTOS kernel with tx_kernel_enter().
*     - tx_kernel_enter() calls tx_application_define() (in app_threadx.c)
*       to create the byte pool, semaphore, and all application threads.
*     - Control never returns from tx_kernel_enter() under normal operation.
*  4. An infinite loop is placed after tx_kernel_enter() as a safety net
*     (should never be reached).
*
* Parameters:
*  void
*
* Return:
*  int  (never reached in practice)
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize all device and board peripherals (clocks, GPIO, SCBs, etc.)
     * Halt on failure since execution cannot continue without proper hardware init. */
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);  /* Fatal: board init failed */
    }

    /* Enable global CPU interrupts so that the RTOS scheduler and
     * SysTick timer can operate correctly. */
    __enable_irq();

    /* Hand over execution to the ThreadX RTOS kernel.
     * Internally this calls tx_application_define() to create all
     * RTOS objects (threads, semaphores, memory pools), then starts
     * the scheduler. This function does not return under normal operation. */
    tx_kernel_enter();

    /* Safety infinite loop - execution should never reach here */
    for (;;)
    {
    }
}
