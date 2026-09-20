/******************************************************************************
* File Name:   main.c
*
* Description: PPCA Core 1 application for threadx RTOS blinky.
*              Toggles GPIO LED every 800ms
*              
*              
*
* Related Document: See README.md
*
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
********************************************************************************/

#include "cy_pdl.h"
#include "cycfg.h"
#include <stdio.h>

/*******************************************************************************
* Macros
********************************************************************************/
#define LOOP_DELAY_800MS 800

/*******************************************************************************
* Global Variables
********************************************************************************/


/*******************************************************************************
* Function Prototypes
********************************************************************************/


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* PPCA Core 1 application:
*    - Toggles shared variable between 0 and 1 every 1 second
*    - Starts with 500ms offset from Core 0 for staggered LED pattern
*    - Main core monitors this variable and controls LED4
*    - LED4 state reflects this core's activity
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    static bool led_toggle = false;
        
    /* Enable global interrupts */
    __enable_irq();

     Cy_SysLib_Delay(500);

     /* Main loop: Toggle shared variable every 1 second */
     for(;;)
     {
            if (led_toggle == false) 
            {
                // Set the LED pin high
                led_toggle = true;
                Cy_PPCA_EPU_PU_T2_Generate_SW_Event(PU_T2_3_GPIO2_HW, PU_T2_3_GPIO2_INDEX, led_toggle);
            } 
            else 
            {
                // Set the LED pin low
                led_toggle = false;
                Cy_PPCA_EPU_PU_T2_Generate_SW_Event(PU_T2_3_GPIO2_HW, PU_T2_3_GPIO2_INDEX, led_toggle);
            }

            // Delay for 800ms
            Cy_SysLib_Delay(LOOP_DELAY_800MS);
       }

}
