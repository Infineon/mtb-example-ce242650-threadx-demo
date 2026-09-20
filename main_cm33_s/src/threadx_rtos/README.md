# ThreadX RTOS for Infineon MCUs 

## Overview 

ThreadX, also known as Azure RTOS in previous branding, is supplied as standard C/ASM source files built along with the other C files in your project. This repository contains a port of the ThreadX kernel for Infineon MCUs based on Cortex&reg;-M33 (CM33). 


## Key Features
- Preemptive Multitasking with priority-based scheduling
- Fast Context Switching and low interrupt latency
- Deterministic Real-Time Behavior
- Minimal Footprint, ideal for embedded systems
- Rich Set of Synchronization Primitives: semaphores, mutexes, message queues, event flags
- Built-in Memory Management
- Highly Portable and Scalable, supporting many processor architectures
- Support for Safety-Critical Certifications (e.g., IEC 61508, ISO 26262)
- Modular Architecture with optional components like FileX, USBX, NetX, and GUIX

## Quick Start 

The quick start guide provides steps to create a simple blinking LED project with a single task using ThreadX.

This quick start guide assumes that the environment is configured to use the board support package (BSP) for your kit and it is included in the project.

Do the following to create a simple ThreadX application:

1. Add ThreadX to the project. For ModusToolbox&trade; software, enable FreeRTOS using the library manager. 

2. Add the RTOS support libraries to the project:
    - [abstraction-rtos](https://github.com/Infineon/abstraction-rtos)
    - [clib-support](https://github.com/Infineon/clib-support)

    **Note:** ModusToolbox&trade; library manager automatically pulls the dependent libraries once the FreeRTOS library is selected.

3. Add `THREADX` to the `COMPONENTS` variable defined in the ModusToolbox&trade; application Makefile:
    ```
    COMPONENTS+=THREADX
    ```
4. Do the following in the *main.c* file:

    1. Include the required header files:

        ```c
        #include "cy_pdl.h"
        #include "cybsp.h"
        #include "tx_api.h"
        ```

    2. Declare thread pool and thread id
        ```c
        static TX_BYTE_POOL tx_app_byte_pool __attribute__((aligned(8)));
        static TX_THREAD led_thread_ptrl __attribute__((aligned(8)));
        ```

    3. Add the function to toggle the LED, assuming it's already configured via `device-configurator`
        ```c
        VOID led_thread_entry(ULONG initial_input)
        {
            (void) initial_input;

            while(1)
            {
                /* Toggle LED */
                Cy_GPIO_Inv(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);
                /* Sleep 500ms */
                tx_thread_sleep(500);
            }
        }
        ```

    4. Add the function that defines threadx applications. This is the function threadx kernels calls before starting the scheduling 
        ``` c
        VOID tx_application_define(VOID *first_unused_memory)
        {
            VOID *memory_ptr;
            CHAR *pointer;

            if (tx_byte_pool_create(&tx_app_byte_pool, "Tx App memory pool", tx_byte_pool_buffer, 2048) != TX_SUCCESS)
            {
                CY_ASSERT(0);
            }

            /* Allocate the stack for Main Thread */
            if (tx_byte_allocate(tx_app_byte_pool, (VOID**) &pointer, 512, TX_NO_WAIT) != TX_SUCCESS)
            {
                return TX_POOL_ERROR;
            }

            /* Create Main Thread. */
            if (tx_thread_create(&led_thread_ptrl, "LED Thread", led_thread_entry, 0, pointer, 512,
                                1, 1, TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
            {
                return TX_THREAD_ERROR;
            }
        }
        ```

    5. Initialize BSP and start the task scheduler:

        ```c
        int main(void)
        {
            cy_rslt_t result;

            /* Initialize the device and board peripherals */
            result = cybsp_init() ;
            if (result != CY_RSLT_SUCCESS)
            {
                CY_ASSERT(0);
            }

            __enable_irq();

            /* start kernel */
            tx_kernel_enter();

            for (;;)
            {
                /* Never returns */
            }
        }
        ```

5. Build the project and program it into the target kit.

6. Observe the LED blinking on the kit.


## Configuration considerations

To add in user configurations of ThreadX, copy the sample *tx_user_sample.h* file from *threadx/Source/common/inc* folder to your project as *tx_user.h* and modify the copied configuration file as needed. 

To enable the user include file, update the project Makefile with the following:
``` Makefile
# Add additional defines to the build process (without a leading -D).
DEFINES=TX_INCLUDE_USER_DEFINE_FILE
```

## More information

- [ThreadX RELEASE.md](./RELEASE.md)
- [ThreadX documentation](https://github.com/eclipse-threadx/rtos-docs)
- [ThreadX homepage](https://threadx.io/)
- [ModusToolbox&trade; software environment, quick start guide, documentation, and videos](https://www.cypress.com/products/modustoolbox-software-environment)
---

© Cypress Semiconductor Corporation (an Infineon Technologies company), 2025.
