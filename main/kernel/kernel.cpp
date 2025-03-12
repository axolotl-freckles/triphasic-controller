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

#include "phases.hpp"

struct KernelArguments {
};

void init_kernel(void* kernel_argp) {
	KernelArguments *kernel_args = (KernelArguments*)kernel_argp;

	init_phases();
	ESP_ERROR_CHECK(ledc_fade_func_install(0));
}

void kernel_loop(void* kernel_argp) {
	// Read sensors
	// Update values
	// Send signals
}