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
bool test_phases(void);

const char  OK_MESSAGE[] = "PASSED!";
const char NOK_MESSAGE[] = "FAILED!";

extern "C" void app_main(void) {
	(void)printf("----TEST MODE----\n");
	ESP_LOGI(TEST_TAG, "Testing!");

	bool phases_ok = test_phases();

	(void)printf("----TESTING FINISHED!----\n");
	while (true) {
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}

bool test_phases(void) {
	(void)printf("--TESTING PHASES AND PWM--\n");
	ESP_LOGI(TEST_TAG, "Initializing phases...");
	esp_timer_handle_t sine_generator_timer_handler;
	bool result = false;
	result = init_phases(&sine_generator_timer_handler);
	(void)printf("PHASE INIT: %s\n", result? OK_MESSAGE:NOK_MESSAGE);
	ESP_ERROR_CHECK(ledc_fade_func_install(0));

	return true;
}

#endif