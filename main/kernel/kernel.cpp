/**
 * @file kernel.cpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-02-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "kernel.hpp"

#include "esp_log.h"

#include "phases.hpp"
#include "sensors.hpp"

const char LOG_TAG[] = "controller_kernel";

// struct KernelArguments {
// };
static volatile float cached_phase_voltage[3] = {0.0f};
static volatile float cached_source_voltage = 0.0f;

static volatile float cached_phase_current[3] = {0.f};
static volatile float cached_source_current = 0.0f;

void init_kernel() {
	// KernelArguments *kernel_args = (KernelArguments*)kernel_argp;

	bool phase_ok = phases::init_phases();
	if (phase_ok) {
		ESP_LOGI(LOG_TAG, "phases ok!");
	}
	else {
		ESP_LOGE(LOG_TAG, "error in phases!!");
		return;
	}
	phases::phase_output_intr(nullptr);
	ESP_ERROR_CHECK(ledc_fade_func_install(0));

	bool sensors_ok = sensors::init_sensors();
	if (sensors_ok) {
		ESP_LOGI(LOG_TAG, "sensors ok!");
	}
	else {
		ESP_LOGE(LOG_TAG, "error in sensors!");
	}
}

void kernel_loop() {
	// Read sensors
	// Update values
	// Send signals
}