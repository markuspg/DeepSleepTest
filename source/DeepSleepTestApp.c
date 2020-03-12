/*
 * Licensee agrees that the example code provided to Licensee has been developed and released by Bosch solely as an example to be used as a potential reference for application development by Licensee.
 * Fitness and suitability of the example code for any use within application developed by Licensee need to be verified by Licensee on its own authority by taking appropriate state of the art actions and measures (e.g. by means of quality assurance measures).
 * Licensee shall be responsible for conducting the development of its applications as well as integration of parts of the example code into such applications, taking into account the state of the art of technology and any statutory regulations and provisions applicable for such applications. Compliance with the functional system requirements and testing there of (including validation of information/data security aspects and functional safety) and release shall be solely incumbent upon Licensee.
 * For the avoidance of doubt, Licensee shall be responsible and fully liable for the applications and any distribution of such applications into the market.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     (1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *     (2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *
 *     (3)The name of the author may not be used to
 *     endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 *  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 *  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#include "DeepSleepTestAppInfo.h"

#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID XDK_APP_MODULE_ID_DEEP_SLEEP_TEST_APP

#include "DeepSleepTestApp.h"
#include "battery_measure_task.h"

#include "BCDS_CmdProcessor.h"
#include "BatteryMonitor.h"
#include "XDK_Utils.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>

#define APP_STARTUP_DELAY UINT32_C(4192)
#define XDK_APP_DELAY UINT32_C(1000)

static CmdProcessor_T * app_cmd_processor = NULL;
static TaskHandle_t battery_measure_task_handle = NULL;

static void create_tasks(void * param1, uint32_t param2);
static void enable_subsystems(void * param1, uint32_t param2);
static void setup_subsystems(void * param1, uint32_t param2);

static void create_tasks(void * param1, uint32_t param2) {
	BCDS_UNUSED(param1);
	BCDS_UNUSED(param2);

	Retcode_T retcode = RETCODE_FAILURE;

	printf("create_tasks \r\n");

	if (pdPASS != xTaskCreate(battery_measure_task, "batt_meas_t",
			BATTERY_MEASURE_TASK_STACK_DEPTH, NULL,
			BATTERY_MEASURE_TASK_PRIO, &battery_measure_task_handle)) {
		retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_OUT_OF_RESOURCES);
	} else {
		retcode = RETCODE_OK;
	}
	if (RETCODE_OK != retcode) {
		printf("create_tasks failed \r\n");
		Retcode_RaiseError(retcode);
		assert(0);
	}
}

static void enable_subsystems(void * param1, uint32_t param2) {
	BCDS_UNUSED(param1);
	BCDS_UNUSED(param2);

	Retcode_T retcode = RETCODE_FAILURE;

	printf("enable_subsystems \r\n");

	retcode = CmdProcessor_Enqueue(app_cmd_processor, create_tasks, NULL,
			UINT32_C(0));

	if (RETCODE_OK != retcode) {
		printf("enable_subsystems failed \r\n");
		Retcode_RaiseError(retcode);
		assert(0);
	}
}

static void setup_subsystems(void * param1, uint32_t param2) {
	BCDS_UNUSED(param1);
	BCDS_UNUSED(param2);

	Retcode_T retcode = RETCODE_FAILURE;

	printf("setup_subsystems \r\n");

	retcode = BatteryMonitor_Init();
	if (RETCODE_OK != retcode) {
		printf("Failed to initialize battery monitor \r\n");
	}

	if (retcode == RETCODE_OK) {
		retcode = CmdProcessor_Enqueue(app_cmd_processor, enable_subsystems,
		NULL, UINT32_C(0));
	}

	if (RETCODE_OK != retcode) {
		printf("setup_subsystems failed \r\n");
		Retcode_RaiseError(retcode);
		assert(0);
	}
}

void DeepSleepTestApp_Init(void * cmd_processor_handle, uint32_t param2) {
	BCDS_UNUSED(param2);

	Retcode_T retcode = RETCODE_FAILURE;

	vTaskDelay(pdMS_TO_TICKS(APP_STARTUP_DELAY));

	printf("DeepSleepTestApp_Init \r\n");

	if (cmd_processor_handle == NULL) {
		printf("DeepSleepTestApp_Init : cmd_processor_handle is NULL \r\n");
		retcode = RETCODE(RETCODE_SEVERITY_ERROR, RETCODE_NULL_POINTER);
	} else {
		app_cmd_processor = (CmdProcessor_T *) cmd_processor_handle;
		retcode = CmdProcessor_Enqueue(app_cmd_processor, setup_subsystems,
		NULL, UINT32_C(0));
	}

	if (RETCODE_OK != retcode) {
		printf("DeepSleepTestApp_Init failed \r\n");
		Retcode_RaiseError(retcode);
		assert(0);
	}
}
