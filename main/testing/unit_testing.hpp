/**
 * @file testing.hpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <functional>

#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_timer.h"

#define TEST_TAG "TESTING"
#define OK_NOK(x)(x?OK_MESSAGE:NOK_MESSAGE)

const char  OK_MESSAGE[] = "OK!";
const char NOK_MESSAGE[] = "NOT OK";

constexpr float EPSILON = 1e-4f;

bool test_phases(void);
bool test_sensors(void);

inline void run_test(
	const char *test_category,
	const char *test_name,
	int &ran_tests, int &passed_tests,
	std::function<bool(void*)> test_fnct,
	void *test_args_p = nullptr
) {
	bool test_ok = test_fnct(test_args_p);
	if (test_ok) {
		ESP_LOGI(TEST_TAG, "%s[%2d] %7s| %s", test_category, ++ran_tests, OK_NOK(test_ok), test_name);
		passed_tests++;
	}
	else {
		ESP_LOGE(TEST_TAG, "%s[%2d] %7s| %s", test_category, ++ran_tests, OK_NOK(test_ok), test_name);
	}
}
