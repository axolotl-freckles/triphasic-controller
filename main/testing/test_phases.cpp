/**
 * @file test_phases.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-03-26
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "unit_testing.hpp"
#include "sdkconfig.h"
#include "../kernel/kernel.hpp"
#include "../kernel/sensors.hpp"
#include "../kernel/phases.hpp"

const char PHASE_TAG[] = "PHASES";

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
	(void)printf("\n--TESTING PHASES AND PWM--\n");

	ESP_LOGI(TEST_TAG, "Initializing phases...");
	run_test(PHASE_TAG, "phase init", n_ran_tests, passed,
		[](void* argp) -> bool {
			bool init_ok = init_phases();
			ledc_fade_func_install(0);
			return init_ok;
		}
	);

	uint32_t theta_int = rad_to_theta_int(M_PI);
	run_test(PHASE_TAG, "theta2int conversion", n_ran_tests, passed,
		[theta_int](void* argp) -> bool {
			bool theta_int_ok = false;
			theta_int_ok = theta_int == (MAX_INT32/2);
			theta_int_ok = theta_int_ok || theta_int == (MAX_INT32/2 - 1);
			theta_int_ok = theta_int_ok || theta_int == (MAX_INT32/2 + 1);
			return theta_int_ok;
		}
	);

	run_test(PHASE_TAG, "LUT idx conversion", n_ran_tests, passed,
		[theta_int](void* argp) -> bool {
			return theta_int_to_lut_idx(theta_int) == (32/2);
		}
	);

	uint32_t sin_output    = 0;
	uint32_t sin_reference = 0;
	run_test(PHASE_TAG, "SIN LUT +in +out", n_ran_tests, passed,
		[&sin_output, &sin_reference](void* argp) -> bool {
			sin_output = sin_lut(rad_to_theta_int(M_PI_2));
			sin_reference = (PWM_MAX_VAL<<16)|(PWM_MAX_VAL);
			return sin_output == sin_reference;
		}
	);
	// (void)printf("     LUT OUT: 0x%08lX\n", sin_output);
	// (void)printf("     LUT REF: 0x%08lX\n", sin_reference);

	run_test(PHASE_TAG, "SIN LUT +in -out", n_ran_tests, passed,
		[&sin_output, &sin_reference](void* argp) -> bool {
			sin_output = sin_lut(rad_to_theta_int(M_PI_2*3.0f));
			sin_reference = (0<<16) | (0);
			return sin_output == sin_reference;
		}
	);
	// (void)printf("     LUT OUT: 0x%08lX\n", sin_output);
	// (void)printf("     LUT REF: 0x%08lX\n", sin_reference);

	run_test(PHASE_TAG, "SIN LUT -in -out", n_ran_tests, passed,
		[&sin_output, &sin_reference](void* argp) -> bool {
			sin_output = sin_lut(rad_to_theta_int(-M_PI_2));
			sin_reference = (0<<16) | (0);
			return sin_output == sin_reference;
		}
	);
	// (void)printf("     LUT OUT: 0x%08lX\n", sin_output);
	// (void)printf("     LUT REF: 0x%08lX\n", sin_reference);

	run_test(PHASE_TAG, "SIN LUT -in +out", n_ran_tests, passed,
		[&sin_output, &sin_reference](void* argp) -> bool {
			sin_output = sin_lut(rad_to_theta_int(-M_PI_2*3));
			sin_reference = (PWM_MAX_VAL<<16) | (PWM_MAX_VAL);
			return sin_output == sin_reference;
		}
	);
	// (void)printf("     LUT OUT: 0x%08lX\n", sin_output);
	// (void)printf("     LUT REF: 0x%08lX\n", sin_reference);

	const float amplitude_setpoint = 0.5f;
	run_test(PHASE_TAG, "amplitude by api", n_ran_tests, passed,
		[amplitude_setpoint](void* argp) -> bool {
			set_amplitude(amplitude_setpoint);
			vTaskDelay(1);
			return std::abs(amplitude_setpoint - get_amplitude()) < EPSILON;
		}
	);
	// (void)printf("     AMPLITUDE VAL: %f\n", get_amplitude());

	run_test(PHASE_TAG, "amplitude by pwm", n_ran_tests, passed,
		[amplitude_setpoint](void* argp) -> bool {
			uint32_t amplitude_dutycycle = ledc_get_duty(LEDC_HIGH_SPEED_MODE, AMPLITUDE_PWM_CHANNEL);
			return amplitude_dutycycle == (uint32_t)(PWM_MAX_VAL*amplitude_setpoint);
		}
	);

	const float frequency_hz_setpoint = 50.0f;
	run_test(PHASE_TAG, "frecuency api", n_ran_tests, passed,
		[frequency_hz_setpoint](void* argp) -> bool {
			set_frequency(frequency_hz_setpoint);
			return std::abs(get_frequency() - frequency_hz_setpoint) < EPSILON;
		}
	);

	const float angular_speed_setpoint = 20*M_TAU;
	run_test(PHASE_TAG, "angular speed api", n_ran_tests, passed,
		[angular_speed_setpoint](void* argp) -> bool {
			set_angular_speed(angular_speed_setpoint);
			return std::abs(get_angular_speed() - angular_speed_setpoint) < EPSILON;
		}
	);

	constexpr int TIME_AVERAGE_N = 1000;
	constexpr double MAX_INTRR_ALLOWED_TIMEus = 100.0f;
	double max_time = 0;
	double total_elapsed_time_us = 0.0f;
	double intrr_average_exec_time = 0.0f;
	phase_output_intr(nullptr);
	run_test(PHASE_TAG, "interrupt exec time", n_ran_tests, passed,
		[
			TIME_AVERAGE_N,
			&max_time,
			&total_elapsed_time_us,
			&intrr_average_exec_time
		](void*argp) -> bool {
			int64_t time_st = 0, time_en = 0;
			for (int i=0; i<TIME_AVERAGE_N; i++) {
				time_st = esp_timer_get_time();
				phase_output_intr(nullptr);
				time_en = esp_timer_get_time();
				int64_t elapsed_time = time_en - time_st;
				if (elapsed_time > max_time)
					max_time = elapsed_time;
				total_elapsed_time_us += time_en - time_st;
			}
			intrr_average_exec_time = total_elapsed_time_us / TIME_AVERAGE_N;
			return intrr_average_exec_time < MAX_INTRR_ALLOWED_TIMEus;
		}
	);
	(void)printf("     AVERAGE ELAPSED TIME: %.3eus\n", intrr_average_exec_time);
	(void)printf("     MAX TIME:             %.3eus\n", max_time);

	run_test(PHASE_TAG, "timer frequency allowed?", n_ran_tests, passed,
		[max_time](void* argp) {
			return max_time < SINE_WAVE_SAMPLE_TIMEus;
		}
	);
	(void)printf("     MIN ALLOWABLE TIMER PERIOD:   %.3eus\n", (double)SINE_WAVE_SAMPLE_TIMEus);
	(void)printf("     MIN TEORETHICAL TIMER PERIOD: %.3eus\n", max_time);

	run_test(PHASE_TAG, "phase start", n_ran_tests, passed,
		[](void*argp) -> bool {
			start_phases();
			vTaskDelay(1);
			return is_active_phases();
		}
	);

	run_test(PHASE_TAG, "phase stop", n_ran_tests, passed,
		[](void* argp) -> bool {
			vTaskDelay(1000/portTICK_PERIOD_MS);
			stop_phases();
			return !is_active_phases();
		}
	);

	run_test(PHASE_TAG, "phase kill", n_ran_tests, passed,
		[](void* argp) -> bool {
			start_phases();
			vTaskDelay(1000/portTICK_PERIOD_MS);
			kill_phases();
			bool stop_ok = !is_active_phases();
			vTaskDelay(1);
			return stop_ok && (get_amplitude() < 1e-6f);
		}
	);
	// (void)printf("     AMPLITUDE VAL: %f\n", get_amplitude());

	run_test(PHASE_TAG, "multicore", n_ran_tests, passed,
		[](void*argp) -> bool {
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
			return true;
		}
	);

	(void)printf("PASSED %2d of %2d tests!\n", passed, n_ran_tests);
	return !(passed < n_ran_tests);
}