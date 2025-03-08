/**
 * @file main.cpp
 * @author ACMAX (aavaloscorrales@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-02-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "sdkconfig.h"
#ifndef CONFIG_UNIT_TEST_MODE

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#define MAIN_TAG "MAIN"

#include "kernel/kernel.hpp"

extern "C" void app_main(void) {
	(void)printf("----MAIN----\n");
	// init_kernel();

	while (true) {
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

#endif