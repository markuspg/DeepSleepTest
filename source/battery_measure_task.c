/*
 * Copyright (c) 2020, Markus Prasser
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the <organization>.
 * 4. Neither the name of the <organization> nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "battery_measure_task.h"

#include "BatteryMonitor.h"
#include "XDK_Storage.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>

#define TASK_DELAY_INTERVAL UINT32_C(600000)

void battery_measure_task(void * task_parameters) {
	BCDS_UNUSED(task_parameters);

	uint32_t battery_voltage_mV;
	TickType_t last_wake_tick = xTaskGetTickCount();
	Retcode_T retcode = RETCODE_OK;
	Storage_Setup_T storage_setup = { .SDCard = true, .WiFiFileSystem = false };

	if (RETCODE_OK != BatteryMonitor_Init()) {
		printf("Failed to initialize battery monitor \r\n");
		assert(0);
	}

	while (1) {
		vTaskDelayUntil(&last_wake_tick, pdMS_TO_TICKS(TASK_DELAY_INTERVAL));

		/* Initialize storage */
		retcode = Storage_Setup(&storage_setup);
		if (retcode != RETCODE_OK) {
			printf("Failed to setup storage \r\n");
			goto close_storage_label;
		}

		retcode = Storage_Enable();
		if (retcode != RETCODE_OK) {
			printf("Failed to enable storage \r\n");
			goto disable_storage_label;
		}

		/* Measure battery voltage */
		retcode = BatteryMonitor_MeasureSignal(&battery_voltage_mV);
		if (retcode != RETCODE_OK) {
			printf("Failed to read battery voltage value \r\n");
			goto disable_storage_label;
		}

		/* Deinitialize storage */
		disable_storage_label: retcode = Storage_Disable(
				STORAGE_MEDIUM_SD_CARD);
		if (retcode != RETCODE_OK) {
			printf("Failed to disable SD card storage \r\n");
		}

		close_storage_label: retcode = Storage_Close();
		if (retcode != RETCODE_OK) {
			printf("Failed to close storage \r\n");
		}
	}
}
