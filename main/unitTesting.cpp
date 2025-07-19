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

inline void report_summary(const char* module, bool result) {
	if (result)
		ESP_LOGI("MODULE", "%s %s", module, OK_NOK(result));
	else
		ESP_LOGE("MODULE", "%s %s", module, OK_NOK(result));
}

extern "C" void app_main(void) {
	(void)printf("----TEST MODE----\n");
	ESP_LOGI(TEST_TAG, "Testing!");

	bool phases_ok = test_phases();
	bool sensors_ok = test_sensors();

	(void)printf("\n----SUMMARY----\n");
	report_summary(" PHASES", phases_ok);
	report_summary("SENSORS", sensors_ok);

	(void)printf("----TESTING FINISHED!----\n");
	while (true) {
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}

#endif