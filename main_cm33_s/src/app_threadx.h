/*******************************************************************************
* File Name: app_threadx.h
*
* Description: Public interface for the ThreadX application layer.
*              Defines all configuration macros (stack sizes, priorities,
*              timing constants, kernel object names) and declares the extern
*              symbols for thread stacks, kernel objects (threads, semaphore,
*              byte pool), and thread entry functions.
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

#ifndef __APP_THREADX_H__
#define __APP_THREADX_H__

#include "tx_port.h"
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cy_pdl.h"    /* Peripheral Driver Library (GPIO, SCB, etc.)    */
#include "cycfg.h"     /* Device Configurator generated peripheral config */
#include "cybsp.h"     /* Board Support Package - BSP aliases and init    */

/* ThreadX RTOS public API */
#include "tx_api.h"


/*******************************************************************************
* Macros - Timing
*******************************************************************************/
/** LED toggle period in RTOS ticks (1 tick = 1 ms -> 1000 ms = 1 s blink rate) */
#define USER_LED_TOGGLE_PERIOD_MS           1000u

/*******************************************************************************
* Macros - Memory
*******************************************************************************/
/** Total size of the static byte pool buffer used for dynamic RTOS allocations */
#define TX_APP_MEM_POOL_SIZE                (8*1024)  /* 8 kB */

/*******************************************************************************
* Macros - Thread Stack Sizes
*******************************************************************************/
/** Stack size for Blinky and Main Task threads (bytes) */
#define THREAD_STACK_SIZE                   1024
/** Stack size for the Main boot thread (bytes) - larger due to printf + init code */
#define THREAD_STACK_SIZE_MAIN              2048

/*******************************************************************************
* Macros - Thread Priorities
*
* ThreadX priority: lower numeric value = higher scheduling priority.
* Priority 0 is the highest possible; TX_MAX_PRIORITIES-1 is the lowest.
*******************************************************************************/
#define THREAD_PRIORITY_MAIN_BOOT           1   /* Highest - used during hardware init at boot  */
#define THREAD_PRIORITY_BLINKY              5   /* Normal  - blinky thread (waits on semaphore) */
#define THREAD_PRIORITY_MAIN_TASK           5   /* Normal  - timing thread (releases semaphore) */
#define THREAD_PRIORITY_MAIN_BACKGROUND     20  /* Lowest  - main thread after init is complete */


/*******************************************************************************
* Global Variables - Thread Stacks (defined in thread_led.c / thread_main.c)
*******************************************************************************/
/* Thread stack arrays - 8-byte aligned as required by Cortex-M ABI */
extern UCHAR thread_blinky_stack[THREAD_STACK_SIZE] __attribute__((aligned(8)));
extern UCHAR thread_main_task_stack[THREAD_STACK_SIZE] __attribute__((aligned(8)));
extern UCHAR thread_main_stack[THREAD_STACK_SIZE_MAIN] __attribute__((aligned(8)));

/** Static backing buffer for the ThreadX byte pool (defined in thread_led.c) */
extern UCHAR byte_pool_buffer[TX_APP_MEM_POOL_SIZE] __attribute__((aligned(8)));


/*******************************************************************************
* Global Variables - ThreadX Kernel Objects (defined in thread_led.c / thread_main.c)
*******************************************************************************/
extern TX_THREAD    thread_blinky;      /* Blinky thread - toggles USER LED             */
extern TX_THREAD    thread_main_task;   /* Main task thread - generates timing signal   */
extern TX_THREAD    thread_main;        /* Main boot thread - HW init then background   */
extern TX_BYTE_POOL my_byte_pool;       /* Byte pool for dynamic memory allocation      */
extern TX_SEMAPHORE led_semaphore;      /* Semaphore: main_task signals blinky to toggle*/

/*******************************************************************************
* Macros - ThreadX Kernel Object Names (used in tx_*_create() calls)
*******************************************************************************/
#define TX_NAME_SEMAPHORE               "LED Semaphore"  /* Name shown in RTOS-aware debuggers */
#define TX_NAME_BYTE_POOL               "Memory Pool"
#define TX_NAME_THREAD_BLINKY           "Blinky"
#define TX_NAME_THREAD_MAIN_TASK        "Main Task"
#define TX_NAME_THREAD_MAIN             "Main Boot"


/*******************************************************************************
* Functions
*******************************************************************************/
/**
 * @brief Creates all ThreadX kernel objects (byte pool, semaphore, threads).
 *        Called from tx_application_define() in app_threadx.c.
 * @return TX_SUCCESS on success; TX_POOL_ERROR or TX_THREAD_ERROR on failure.
 */
UINT App_ThreadX_Init(VOID);

/**
 * @brief Blinky thread entry - waits on led_semaphore, toggles USER LED.
 *        Defined in thread_led.c.
 */
VOID thread_blinky_entry(ULONG input);

/**
 * @brief Main task thread entry - sleeps USER_LED_TOGGLE_PERIOD_MS, releases semaphore.
 *        Defined in thread_led.c.
 */
VOID thread_main_task_entry(ULONG input);

/**
 * @brief Main boot thread entry - initializes UART, boots PPCA cores, then idles.
 *        Defined in thread_main.c.
 */
VOID thread_main_entry(ULONG input);

#ifdef __cplusplus
}
#endif
#endif /* __APP_THREADX_H__ */
