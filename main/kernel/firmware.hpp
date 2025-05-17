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

#include "firmware_types.hpp"

// struct KernelArguments;

void init_kernel();
void kernel_loop();

float get_voltage(PhaseSelector phase);
float get_voltage(void);
float get_current(PhaseSelector phase);
float get_current(void);

void set_frecuency(float frequency_hz);
