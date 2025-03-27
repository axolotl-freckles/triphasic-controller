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

#include "testing/unit_testing.hpp"

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

#endif