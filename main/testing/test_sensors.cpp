/**
 * @file test_sensors.cpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-04-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "unit_testing.hpp"

#include "../kernel/sensors.hpp"

static const char TEST_CATEGORY[] = "SENSORS";

bool test_sensors(void) {
	int ran_tests    = 0;
	int passed_tests = 0;
	(void)printf("\n--TESTING SENSORS--\n");

	run_test(TEST_CATEGORY, "init", ran_tests, passed_tests,
		[](void* argp) -> bool {
			return init_sensors();
		}
	);

	(void)printf("PASSED %2d of %2d tests!\n", passed_tests, ran_tests);
	return ran_tests == passed_tests;
}
