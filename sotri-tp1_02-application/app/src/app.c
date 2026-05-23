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
/* Project includes */
#include "main.h"
#include "cmsis_os.h"

/* Demo includes */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes */
#include "board.h"
#include "app_it.h"
#include "task_btn.h"
#include "task_led.h"

/********************** macros and definitions *******************************/
#define G_APP_CNT_INI					0ul
#define G_APP_TASK_CNT_INI				0ul
#define G_APP_TICK_CNT_INI				0ul
#define G_TASK_IDLE_CNT_INI				0ul
#define G_APP_STACK_OVERFLOW_CNT_INI	0ul

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
const char *p_app	= "RTOS - Event-Triggered Systems (ETS)";
const char *p_app_	= "seo-tp1_01-application: Demo Code";
const char *p_app__	= "(Source => CESE - Sistemas Operativos de Tiempo Real)";

/********************** external data declaration ****************************/
uint32_t g_app_cnt;
uint32_t g_app_task_cnt;
uint32_t g_app_tick_cnt;
uint32_t g_task_idle_cnt;
uint32_t g_app_stack_overflow_cnt;

/* Declare a variable of type QueueHandle_t. This is used to reference queues*/

/* Declare a variable of type xSemaphoreHandle (binary or counting) or mutex. 
 * This is used to reference the semaphore that is used to synchronize a thread
 * with other thread or to ensure mutual exclusive access to...*/

/* Declare a variable of type TaskHandle_t. This is used to reference threads. */
TaskHandle_t h_task_btn;
TaskHandle_t h_task_led;

/********************** external functions definition ************************/
void app_init(void)
{
	/*  Declare & Initialize App variables */
	g_app_cnt = G_APP_CNT_INI;
	g_app_task_cnt = G_APP_TASK_CNT_INI;
	g_app_tick_cnt = G_APP_TICK_CNT_INI;
	g_task_idle_cnt = G_TASK_IDLE_CNT_INI;
	g_app_stack_overflow_cnt = G_APP_STACK_OVERFLOW_CNT_INI;

	/* Print out: Application Initialized */
	LOGGER_INFO(" ");
	LOGGER_INFO("%s is running - Tick [mS] = %lu", GET_NAME(app_init), xTaskGetTickCount());

	LOGGER_INFO(" %s is a %s", GET_NAME(app), p_app);
	LOGGER_INFO(" %s is a %s", GET_NAME(app), p_app_);
	LOGGER_INFO(" %s is a %s", GET_NAME(app), p_app__);

    /* Before a queue or semaphore (binary or counting) or mutex is used it must 
     * be explicitly created */

    /* Check the queue or semaphore (binary or counting) or mutex was created 
     * successfully. */

    /* Add queue or semaphore (binary or counting) or mutex to registry. */

	/* Add threads, ... */
    BaseType_t ret;

    /* Task BTN thread at priority 1 */
    ret = xTaskCreate(task_btn,							/* Pointer to the function thats implement the task. */
					  "Task BTN",						/* Text name for the task. This is to facilitate debugging only. */
					  (2 * configMINIMAL_STACK_SIZE),	/* Stack depth in words. */
					  NULL,								/* We are not using the task parameter. */
					  (tskIDLE_PRIORITY + 1ul),			/* This task will run at priority 1. */
					  &h_task_btn);						/* We are using a variable as task handle. */

    /* Check the thread was created successfully. */
    configASSERT(pdPASS == ret);

    /* Task LED thread at priority 1 */
    ret = xTaskCreate(task_led,							/* Pointer to the function thats implement the task. */
					  "Task LED",						/* Text name for the task. This is to facilitate debugging only. */
					  (2 * configMINIMAL_STACK_SIZE),	/* Stack depth in words. */
					  NULL,								/* We are not using the task parameter. */
					  (tskIDLE_PRIORITY + 1ul),			/* This task will run at priority 1. */
					  &h_task_led);						/* We are using a variable as task handle. */

    /* Check the thread was created successfully. */
    configASSERT(pdPASS == ret);

    /* Total amount of heap space that remains unallocated. Is also available
     * with xFreeBytesRemaining variable for heap management schemes 2 to 5.
     * Memory array used by heap_4 is specified as:
     * uint8_t ucHeap[configTOTAL_HEAP_SIZE]; */
    ret = xPortGetFreeHeapSize();

    /* There is no dedicated list for task in Running mode (as we have only
     * one task in this state at the moment), but the currently run task ID
     * is stored in variable pxCurrentTCB */

  	/* Application Interrupts Init */
	app_it_init();

	/* Init Cycle Counter */
	cycle_counter_init();
}

/********************** end of file ******************************************/
