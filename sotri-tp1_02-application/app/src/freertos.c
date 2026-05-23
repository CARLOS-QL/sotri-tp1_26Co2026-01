/*
 * Copyright (c) 2026 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 */

/********************** inclusions *******************************************/
/* Project includes. */
#include "main.h"
#include "cmsis_os.h"

/* Demo includes. */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes. */
#include "board.h"
#include "app.h"

/********************** macros and definitions *******************************/

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

/********************** external data declaration *****************************/

/********************** external functions definition ************************/
/* Hook Functions */
void vApplicationIdleHook(void)
{
	/* The idle task can optionally call an application defined hook (or callback)
	   function - the idle hook. The idle task runs at the very lowest priority,
	   so such an idle hook function will only get executed when there are no tasks
	   of higher priority that are able to run. This makes the idle hook function
	   an ideal place to put the processor into a low power state - providing an
	   automatic power saving whenever there is no processing to be performed.
	   The idle hook will only get called if configUSE_IDLE_HOOK is set to 1
	   https://www.freertos.org/a00016.html
	   The idle hook is called repeatedly as long as the idle task is running. It
	   is paramount that the idle hook function does not call any API functions
	   that could cause it to block.*/

	/* Update Task Idle Counter */
	g_task_idle_cnt++;
}

void vApplicationTickHook(void)
{
	/* The tick interrupt can optionally call an application defined hook (or callback)
	   function - the tick hook.
	   The tick hook will only get called if configUSE_TICK_HOOK is set to 1
	   https://www.freertos.org/a00016.html
	   vApplicationTickHook() executes from within an ISR so must be very short, not use
	   much stack, and not call any API functions that don't end in "FromISR" or "FROM_ISR".*/

	/* Update Application Tick Counter */
	g_app_tick_cnt++;
}

void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
	/* Run time stack overflow checking is performed if
	   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
	   called if a stack overflow is detected.
	   https://www.freertos.org/Stacks-and-stack-overflow-checking.html */

    taskENTER_CRITICAL();
    configASSERT( 0 );   /* hang the execution for debugging purposes */
    taskEXIT_CRITICAL();

	/* Update Application Stack Overflow Counter */
    g_app_stack_overflow_cnt++;
}

/********************** end of file ******************************************/
