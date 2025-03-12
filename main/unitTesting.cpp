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
#include "esp_timer.h"
#define TEST_TAG "TESTING"

#include "kernel/kernel.hpp"
#include "kernel/sensors.hpp"

#include "kernel/phases.hpp"

#define OK_NOK(x)(x?OK_MESSAGE:NOK_MESSAGE)

constexpr float EPSILON = 1e-4f;

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
	int n_ran_tests = 0;
	(void)printf("--TESTING PHASES AND PWM--\n");

	ESP_LOGI(TEST_TAG, "Initializing phases...");
	bool init_ok = init_phases();
	(void)printf("[%2d] PHASE INIT: %s\n", ++n_ran_tests, OK_NOK(init_ok));
	passed += init_ok;

	uint32_t sin_output = sin_lut(M_PI_2);
	uint32_t sin_reference = (0<<16) | (PWM_MAX_VAL);
	bool sin_positive_out_ok = sin_output == sin_reference;
	(void)printf("[%2d] SIN FUNC POSITIVE OUT: %s\n", ++n_ran_tests, OK_NOK(sin_positive_out_ok));
	passed += sin_positive_out_ok;
	
	sin_output = sin_lut(M_PI_2*3.0f);
	sin_reference = (PWM_MAX_VAL<<16) | (0);
	bool sin_negative_out_ok = sin_output == sin_reference;
	(void)printf("[%2d] SIN FUNC NEGATIVE OUT: %s\n", ++n_ran_tests, OK_NOK(sin_negative_out_ok));
	passed += sin_negative_out_ok;
	
	sin_output = sin_lut(-M_PI_2);
	sin_reference = (PWM_MAX_VAL<<16) | (0);
	bool sin_negative_in_ok = sin_output == sin_reference;
	(void)printf("[%2d] SIN FUNC NEGATIVE IN NEGATIVE OUT: %s\n", ++n_ran_tests, OK_NOK(sin_negative_in_ok));
	passed += sin_negative_in_ok;
	
	sin_output = sin_lut(-M_PI_2*3);
	sin_reference = (0<<16) | (PWM_MAX_VAL);
	sin_negative_in_ok = sin_output == sin_reference;
	(void)printf("[%2d] SIN FUNC NEGATIVE IN POSITIVE OUT: %s\n", ++n_ran_tests, OK_NOK(sin_negative_in_ok));
	passed += sin_negative_in_ok;

	const float amplitude_setpoint = 0.5f;
	set_amplitude(amplitude_setpoint);
	bool amplitude_ok = amplitude_setpoint == get_amplitude();
	(void)printf("[%2d] AMPLITUDE API: %s\n", ++n_ran_tests, OK_NOK(amplitude_ok));
	passed += amplitude_ok;

	uint32_t amplitude_dutycycle = ledc_get_duty(LEDC_HIGH_SPEED_MODE, AMPLITUDE_PWM_CHANNEL);
	amplitude_ok = amplitude_dutycycle == (PWM_MAX_VAL/2);
	(void)printf("[%2d] AMPLITUDE PWM: %s\n", ++n_ran_tests, OK_NOK(amplitude_ok));
	passed += amplitude_ok;

	const float frequency_hz_setpoint = 50.0f;
	set_frequency(frequency_hz_setpoint);
	bool frequency_ok = std::abs(get_frequency() - frequency_hz_setpoint) < EPSILON;
	(void)printf("[%2d] FREQUENCY: %s\n", ++n_ran_tests, OK_NOK(frequency_ok));
	passed += frequency_ok;

	const float angular_speed_setpoint = 20*M_TAU;
	set_angular_speed(angular_speed_setpoint);
	bool angular_speed_ok = std::abs(get_angular_speed() - angular_speed_setpoint) < EPSILON;
	(void)printf("[%2d] ANGULAR SPEED: %s\n", ++n_ran_tests, OK_NOK(angular_speed_ok));
	passed += angular_speed_ok;

	constexpr int TIME_AVERAGE_N = 500;
	int64_t time_st = 0, time_en = 0;
	double total_elapsed_time_us = 0.0f;
	for (int i=0; i<TIME_AVERAGE_N; i++) {
		time_st = esp_timer_get_time();
		phase_output_intr(nullptr);
		time_en = esp_timer_get_time();
		total_elapsed_time_us += time_en - time_st;
	}
	double intrr_average_exec_time = total_elapsed_time_us / TIME_AVERAGE_N;
	constexpr double MAX_INTRR_ALLOWED_TIMEus = 100.0f;
	bool intrr_exec_time_ok = intrr_average_exec_time < MAX_INTRR_ALLOWED_TIMEus;
	(void)printf("[%2d] EXEC TIME: %s\n", ++n_ran_tests, OK_NOK(intrr_exec_time_ok));
	(void)printf("     ELAPSED TIME: %Eus\n", intrr_average_exec_time);
	passed += intrr_exec_time_ok;

	start_phases();
	bool start_ok = is_active_phases();
	(void)printf("[%2d] PHASE START: %s\n", ++n_ran_tests, OK_NOK(start_ok));
	passed += start_ok;

	stop_phases();
	bool stop_ok = !is_active_phases();
	(void)printf("[%2d] PHASE STOP: %s\n", ++n_ran_tests, OK_NOK(stop_ok));
	passed += stop_ok;

	start_phases();
	kill_phases();
	stop_ok = !is_active_phases();
	bool kill_ok = stop_ok && (get_amplitude() < 1e-6f);
	(void)printf("[%2d] PHASE KILL: %s\n", ++n_ran_tests, OK_NOK(kill_ok));
	passed += kill_ok;

	(void)printf("PASSED %d of %d tests!\n", passed, n_ran_tests);
	return !(passed < n_ran_tests);
}

#endif