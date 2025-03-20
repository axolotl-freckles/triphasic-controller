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

const char LOG_TAG[] = "controller_kernel";

struct KernelArguments {
};

void init_kernel(void* kernel_argp) {
	KernelArguments *kernel_args = (KernelArguments*)kernel_argp;

	bool phase_ok = init_phases();
	if (phase_ok) {
		ESP_LOGI(LOG_TAG, "phases ok!");
	}
	else {
		ESP_LOGE(LOG_TAG, "error in phases!!");
		return;
	}
	phase_output_intr(nullptr);
	ESP_ERROR_CHECK(ledc_fade_func_install(0));
}

void kernel_loop(void* kernel_argp) {
	// Read sensors
	// Update values
	// Send signals
}