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
#include "task_led_attribute.h"

/********************** macros and definitions *******************************/
#define G_TASK_LED_CNT_INI	0ul

#define DEL_LED_MIN			0ul
#define DEL_LED_MED			250ul
#define DEL_LED_MAX			500ul

/********************** internal data declaration ****************************/
task_led_dta_t task_led_dta = {
		false, EV_LED_OFF, ST_LED_OFF, DEL_LED_MIN,
		LD2_GPIO_Port, LD2_Pin
};

/********************** internal functions declaration ***********************/
void task_led_statechart(void);

/********************** internal data definition *****************************/

/********************** external data declaration ****************************/
uint32_t g_task_led_cnt;

/********************** external functions definition ************************/
/* Task LED thread */
void task_led(void *parameters)
{
	/*  Declare & Initialize Task Function variables */
	g_task_led_cnt = G_TASK_LED_CNT_INI;

	/* Print out: Task Initialized */
	LOGGER_INFO(" ");
	LOGGER_INFO("%s is running - Tick [mS] = %3d", pcTaskGetName(NULL), (int)xTaskGetTickCount());

	HAL_GPIO_WritePin(task_led_dta.gpio_port, task_led_dta.pin, LED_OFF);

	/* As per most tasks, this task is implemented in an infinite loop. */
	for (;;)
	{
		/* Update Task Counter */
		g_task_led_cnt++;

		/* Run Task Statechart */
    	task_led_statechart();
	}
}

void task_led_statechart(void)
{
	switch (task_led_dta.state)
	{
		case ST_LED_OFF:

			if ((true == task_led_dta.flag) && (EV_LED_BLINK == task_led_dta.event))
			{
				/* Print out: Task execution */
				LOGGER_INFO(" %s - LED BLINK", pcTaskGetName(NULL));

				task_led_dta.flag = false;
				task_led_dta.tick = xTaskGetTickCount();
				task_led_dta.state = ST_LED_BLINK;
				HAL_GPIO_WritePin(task_led_dta.gpio_port, task_led_dta.pin, LED_ON);
			}

			break;

		case ST_LED_BLINK:

			if ((true == task_led_dta.flag) && (EV_LED_OFF == task_led_dta.event))
			{
				/* Print out: Task execution */
				LOGGER_INFO(" %s - LED OFF", pcTaskGetName(NULL));

				task_led_dta.flag = false;
				task_led_dta.state = ST_LED_OFF;
				HAL_GPIO_WritePin(task_led_dta.gpio_port, task_led_dta.pin, LED_OFF);
			}
			else
			{
				if (DEL_LED_MAX <= (xTaskGetTickCount() - task_led_dta.tick))
				{
					task_led_dta.tick = xTaskGetTickCount();
					HAL_GPIO_TogglePin(task_led_dta.gpio_port, task_led_dta.pin);
				}
			}

			break;

		default:

			task_led_dta.flag = false;
			task_led_dta.event = EV_LED_OFF;
			task_led_dta.state = ST_LED_OFF;
			task_led_dta.tick  = xTaskGetTickCount();
			HAL_GPIO_WritePin(task_led_dta.gpio_port, task_led_dta.pin, LED_OFF);

			break;
	}
}

/********************** end of file ******************************************/
