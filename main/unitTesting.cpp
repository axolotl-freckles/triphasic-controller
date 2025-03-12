/**
 * @file unitTesting.cpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-03-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "sdkconfig.h"
#ifdef CONFIG_UNIT_TEST_MODE

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#define TEST_TAG "TESTING"

#include "kernel/kernel.hpp"
#include "kernel/sensors.hpp"

#include "kernel/phases.hpp"

#define OK_NOK(x)(x?OK_MESSAGE:NOK_MESSAGE)

bool test_phases(void);

const char  OK_MESSAGE[] = "OK!";
const char NOK_MESSAGE[] = "NOT OK";

extern "C" void app_main(void) {
	(void)printf("----TEST MODE----\n");
	ESP_LOGI(TEST_TAG, "Testing!");

	bool phases_ok = test_phases();
	if (phases_ok)
		ESP_LOGI(TEST_TAG, "PHASES %s\n", OK_NOK(phases_ok));
		else
		ESP_LOGE(TEST_TAG, "PHASES %s\n", OK_NOK(phases_ok));

	(void)printf("----TESTING FINISHED!----\n");
	while (true) {
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}

bool test_phases(void) {
	int passed = 0;
	int tested = 9;
	(void)printf("--TESTING PHASES AND PWM--\n");

	ESP_LOGI(TEST_TAG, "Initializing phases...");
	bool init_ok = init_phases();
	passed += init_ok;
	(void)printf("PHASE INIT: %s\n", init_ok? OK_MESSAGE:NOK_MESSAGE);

	start_phases();
	bool start_ok = is_active_phases();
	passed += start_ok;
	(void)printf("PHASE START: %s\n", start_ok? OK_MESSAGE:NOK_MESSAGE);

	stop_phases();
	bool stop_ok = !is_active_phases();
	passed += stop_ok;
	(void)printf("PHASE STOP: %s\n", stop_ok? OK_MESSAGE:NOK_MESSAGE);

	start_phases();
	kill_phases();
	stop_ok = !is_active_phases();
	bool kill_ok = stop_ok && (get_amplitude() < 1e-6f);
	passed += kill_ok;
	(void)printf("PHASE KILL: %s\n", kill_ok? OK_MESSAGE:NOK_MESSAGE);


	(void)printf("PASSED %d of %d tests!\n", passed, tested);
	return !(passed < tested);
}

#endif