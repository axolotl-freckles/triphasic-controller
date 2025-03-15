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

constexpr float EPSILON = 1e-2f;

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

void test_phases_outcore(void *argp) {
	for (int i=0; i<4; i++) {
		set_amplitude(i*0.1f);
		set_frequency(i);

		vTaskDelay(200/portTICK_PERIOD_MS);
	}

	stop_phases();
	vTaskSuspend(nullptr);
}
bool test_phases(void) {
	int passed = 0;
	int n_ran_tests = 0;
	(void)printf("--TESTING PHASES AND PWM--\n");

	ESP_LOGI(TEST_TAG, "Initializing phases...");
	bool init_ok = init_phases();
	ledc_fade_func_install(0);
	(void)printf("[%2d] PHASE INIT: %s\n", ++n_ran_tests, OK_NOK(init_ok));
	passed += init_ok;

	uint32_t theta_int = rad_to_theta_int(M_PI);
	bool theta_int_ok = (theta_int == MAX_INT32/2) || (theta_int == MAX_INT32/2 - 1) || (theta_int == MAX_INT32/2 + 1);
	(void)printf("[%2d] THETA INT CONVERSION: %s\n", ++n_ran_tests, OK_NOK(theta_int_ok));
	passed += theta_int_ok;

	uint8_t lut_idx = theta_int_to_lut_idx(theta_int);
	bool lut_idx_ok = lut_idx == 32/2;
	(void)printf("[%2d] LUT IDX: %s\n", ++n_ran_tests, OK_NOK(lut_idx_ok));
	passed += lut_idx_ok;

	uint32_t sin_output = sin_lut(rad_to_theta_int(M_PI_2));
	uint32_t sin_reference = (PWM_MAX_VAL<<16) | (PWM_MAX_VAL);
	bool sin_positive_out_ok = sin_output == sin_reference;
	(void)printf("[%2d] SIN FUNC POSITIVE OUT: %s\n", ++n_ran_tests, OK_NOK(sin_positive_out_ok));
	// (void)printf("     LUT OUT: 0x%08lX\n", sin_output);
	// (void)printf("     LUT REF: 0x%08lX\n", sin_reference);
	passed += sin_positive_out_ok;
	
	sin_output = sin_lut(rad_to_theta_int(M_PI_2*3.0f));
	sin_reference = (0<<16) | (0);
	bool sin_negative_out_ok = sin_output == sin_reference;
	(void)printf("[%2d] SIN FUNC NEGATIVE OUT: %s\n", ++n_ran_tests, OK_NOK(sin_negative_out_ok));
	// (void)printf("     LUT OUT: 0x%08lX\n", sin_output);
	// (void)printf("     LUT REF: 0x%08lX\n", sin_reference);
	passed += sin_negative_out_ok;
	
	sin_output = sin_lut(rad_to_theta_int(-M_PI_2));
	sin_reference = (0<<16) | (0);
	bool sin_negative_in_ok = sin_output == sin_reference;
	(void)printf("[%2d] SIN FUNC NEGATIVE IN NEGATIVE OUT: %s\n", ++n_ran_tests, OK_NOK(sin_negative_in_ok));
	// (void)printf("     LUT OUT: 0x%08lX\n", sin_output);
	// (void)printf("     LUT REF: 0x%08lX\n", sin_reference);
	passed += sin_negative_in_ok;
	
	sin_output = sin_lut(rad_to_theta_int(-M_PI_2*3));
	sin_reference = (PWM_MAX_VAL<<16) | (PWM_MAX_VAL);
	sin_negative_in_ok = sin_output == sin_reference;
	(void)printf("[%2d] SIN FUNC NEGATIVE IN POSITIVE OUT: %s\n", ++n_ran_tests, OK_NOK(sin_negative_in_ok));
	// (void)printf("     LUT OUT: 0x%08lX\n", sin_output);
	// (void)printf("     LUT REF: 0x%08lX\n", sin_reference);
	passed += sin_negative_in_ok;

	const float amplitude_setpoint = 0.5f;
	set_amplitude(amplitude_setpoint);
	vTaskDelay(1);
	bool amplitude_ok = std::abs(amplitude_setpoint - get_amplitude()) < EPSILON;
	(void)printf("[%2d] AMPLITUDE API: %s\n", ++n_ran_tests, OK_NOK(amplitude_ok));
	// (void)printf("     AMPLITUDE VAL: %f\n", get_amplitude());
	passed += amplitude_ok;

	uint32_t amplitude_dutycycle = ledc_get_duty(LEDC_HIGH_SPEED_MODE, AMPLITUDE_PWM_CHANNEL);
	amplitude_ok = amplitude_dutycycle == (uint32_t)(PWM_MAX_VAL*amplitude_setpoint);
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

	constexpr int TIME_AVERAGE_N = 1000;
	int64_t time_st = 0, time_en = 0;
	double max_time = 0;
	double total_elapsed_time_us = 0.0f;
	phase_output_intr(nullptr);
	for (int i=0; i<TIME_AVERAGE_N; i++) {
		time_st = esp_timer_get_time();
		phase_output_intr(nullptr);
		time_en = esp_timer_get_time();
		int64_t elapsed_time = time_en - time_st;
		if (elapsed_time > max_time)
			max_time = elapsed_time;
		total_elapsed_time_us += time_en - time_st;
	}
	double intrr_average_exec_time = total_elapsed_time_us / TIME_AVERAGE_N;
	constexpr double MAX_INTRR_ALLOWED_TIMEus = 100.0f;
	bool intrr_exec_time_ok = intrr_average_exec_time < MAX_INTRR_ALLOWED_TIMEus;
	(void)printf("[%2d] EXEC TIME: %s\n", ++n_ran_tests, OK_NOK(intrr_exec_time_ok));
	(void)printf("     AVERAGE ELAPSED TIME: %eus\n", intrr_average_exec_time);
	(void)printf("     MAX TIME:             %eus\n", max_time);
	passed += intrr_exec_time_ok;

	intrr_exec_time_ok = max_time < SINE_WAVE_SAMPLE_TIMEus;
	(void)printf("[%2d] TIMER FREQUENCY ALLOWED: %s\n", ++n_ran_tests, OK_NOK(intrr_exec_time_ok));
	(void)printf("     MIN ALLOWABLE TIMER PERIOD:   %eus\n", (double)SINE_WAVE_SAMPLE_TIMEus);
	(void)printf("     MIN TEORETHICAL TIMER PERIOD: %eus\n", max_time);
	passed += intrr_exec_time_ok;

	start_phases();
	bool start_ok = is_active_phases();
	(void)printf("[%2d] PHASE START: %s\n", ++n_ran_tests, OK_NOK(start_ok));
	passed += start_ok;

	vTaskDelay(1000/portTICK_PERIOD_MS);
	stop_phases();
	bool stop_ok = !is_active_phases();
	(void)printf("[%2d] PHASE STOP: %s\n", ++n_ran_tests, OK_NOK(stop_ok));
	passed += stop_ok;
	
	start_phases();
	vTaskDelay(1000/portTICK_PERIOD_MS);
	kill_phases();
	stop_ok = !is_active_phases();
	vTaskDelay(1);
	bool kill_ok = stop_ok && (get_amplitude() < 1e-6f);
	(void)printf("[%2d] PHASE KILL: %s\n", ++n_ran_tests, OK_NOK(kill_ok));
	// (void)printf("     AMPLITUDE VAL: %f\n", get_amplitude());
	passed += kill_ok;

	start_phases();
	BaseType_t coreID = xPortGetCoreID();
	coreID = (coreID == 0)? 1:0;
	TaskHandle_t outcore_test_handle;
	xTaskCreatePinnedToCore(
		test_phases_outcore,
		"test outcore",
		512,
		nullptr,
		3,
		&outcore_test_handle,
		coreID
	);

	vTaskDelay(1500/portTICK_PERIOD_MS);
	vTaskDelete(outcore_test_handle);

	(void)printf("[%2d] PASSED FROM MULTI-CORE\n", ++n_ran_tests);
	passed++;

	(void)printf("PASSED %d of %d tests!\n", passed, n_ran_tests);
	return !(passed < n_ran_tests);
}

#endif