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
#include "app.h"
#include "task_btn_attribute.h"
#include "task_led_attribute.h"
#include "task_led_interface.h"

/********************** macros and definitions *******************************/
#define G_TASK_BTN_CNT_INI	0ul

#define DEL_BTN_MIN			0ul
#define DEL_BTN_MED			25ul
#define DEL_BTN_MAX			50ul

#define EV_SYS_IDLE			0ul
#define EV_SYS_LOOP_DET		1ul

/********************** internal data declaration ****************************/
task_btn_dta_t task_btn_dta = {
		EV_BTN_UP, ST_BTN_UP, DEL_BTN_MIN,
		B1_GPIO_Port, B1_Pin
};

/********************** internal functions declaration ***********************/
void task_btn_statechart(void);

/********************** internal data definition *****************************/

/********************** external data declaration ****************************/
uint32_t g_task_btn_cnt;

/********************** external functions definition ************************/
/* Task BTN thread */
void task_btn(void *parameters)
{
	/*  Declare & Initialize Task Function variables */
	g_task_btn_cnt = G_TASK_BTN_CNT_INI;

	/* Print out: Task Initialized */
	LOGGER_INFO(" ");
	LOGGER_INFO("%s is running - Tick [mS] = %3d", pcTaskGetName(NULL), (int)xTaskGetTickCount());

	/* As per most tasks, this task is implemented in an infinite loop. */
	for (;;)
	{
		/* Update Task Counter */
		g_task_btn_cnt++;
		
		/* Run Task Statechart */
    	task_btn_statechart();
	}
}

void task_btn_statechart(void)
{
	/* Get Events to excite Task */
	if (BTN_PRESSED == HAL_GPIO_ReadPin(task_btn_dta.gpio_port, task_btn_dta.pin))
	{
		task_btn_dta.event = EV_BTN_DOWN;
	}
	else
	{
		task_btn_dta.event = EV_BTN_UP;
	}

	/* Run to Completion Statechart */
	switch (task_btn_dta.state)
	{
		case ST_BTN_UP:

			if (EV_BTN_DOWN == task_btn_dta.event)
			{
				task_btn_dta.tick = xTaskGetTickCount();
				task_btn_dta.state = ST_BTN_FALLING;
			}

			break;

		case ST_BTN_FALLING:

			if (DEL_BTN_MAX <= (xTaskGetTickCount() - task_btn_dta.tick))
			{
				if (EV_BTN_DOWN == task_btn_dta.event)
				{
					/* Print out: Task execution */
					LOGGER_INFO(" %s - BTN PRESSED", pcTaskGetName(NULL));

					put_event_task_led(EV_LED_BLINK);
					task_btn_dta.state = ST_BTN_DOWN;
				}
				else
				{
					task_btn_dta.state = ST_BTN_UP;
				}
			}

			break;

		case ST_BTN_DOWN:

			if (EV_BTN_UP == task_btn_dta.event)
			{
				task_btn_dta.tick = xTaskGetTickCount();
				task_btn_dta.state = ST_BTN_RISING;
			}

			break;

		case ST_BTN_RISING:

			if (DEL_BTN_MAX <= (xTaskGetTickCount() - task_btn_dta.tick))
			{
				if (EV_BTN_UP == task_btn_dta.event)
				{
					/* Print out: Task execution */
					LOGGER_INFO(" %s - BTN HOVER", pcTaskGetName(NULL));

					put_event_task_led(EV_LED_OFF);
					task_btn_dta.state = ST_BTN_UP;
				}
				else
				{
					task_btn_dta.state = ST_BTN_DOWN;
				}
			}

			break;

		default:

			task_btn_dta.tick  = xTaskGetTickCount();
			task_btn_dta.state = ST_BTN_UP;
			task_btn_dta.event = EV_BTN_UP;

			break;
	}

}

/********************** end of file ******************************************/
