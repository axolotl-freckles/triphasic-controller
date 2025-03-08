/**
 * @file kernel.hpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-02-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include "sdkconfig.h"

#include "esp_timer.h"
#include "freertos/FreeRtos.h"

struct KernelArguments;

void init_kernel(void* kernel_argp);
void kernel_loop(void* kernel_argp);
