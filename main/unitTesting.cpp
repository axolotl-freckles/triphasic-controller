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

extern "C" void app_main(void) {
	ESP_LOGI(TEST_TAG, "Testing!");
	(void)printf("----TEST MODE----\n");

	bool phases_ok = test_phases();

	while (true) {
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}

bool test_phases(void) {
	ESP_LOGI(TEST_TAG, "Initializing phases...");
	esp_timer_handle_t sine_generator_timer_handler;
	init_phases(&sine_generator_timer_handler);
	ESP_ERROR_CHECK(ledc_fade_func_install(0));

	return true;
}

#endif