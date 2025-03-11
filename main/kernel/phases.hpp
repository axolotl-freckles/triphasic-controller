/**
 * @file phases.hpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-02-21
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include "math.h"

#include "sdkconfig.h"

#include "esp_timer.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"

constexpr float M_TAU = M_PI*2;
constexpr uint32_t SINE_WAVE_SAMPLE_TIMEus = CONFIG_SINE_WAVE_SAMPLE_TIMEus;
constexpr float    SINE_WAVE_SAMPLE_TIMEs  = SINE_WAVE_SAMPLE_TIMEus*1e-6f;
constexpr ledc_timer_t PWM_TIMER_ID = LEDC_TIMER_0;
constexpr uint32_t PWM_FREQUENCY_Hz = 2000;
constexpr int      PWM_RESOLUTION   = 8;
constexpr uint32_t PWM_MAX_VAL = (1<<PWM_RESOLUTION) - 1;

constexpr ledc_channel_t AMPLITUDE_PWM_CHANNEL = LEDC_CHANNEL_0;
constexpr ledc_channel_t A_HIGH_CHANNEL = LEDC_CHANNEL_1;
constexpr ledc_channel_t A_LOW_CHANNEL  = LEDC_CHANNEL_2;
constexpr ledc_channel_t B_HIGH_CHANNEL = LEDC_CHANNEL_3;
constexpr ledc_channel_t B_LOW_CHANNEL  = LEDC_CHANNEL_4;
constexpr ledc_channel_t C_HIGH_CHANNEL = LEDC_CHANNEL_5;
constexpr ledc_channel_t C_LOW_CHANNEL  = LEDC_CHANNEL_6;

constexpr int A_HIGH_GPIO = 14;
constexpr int A_LOW_GPIO  = 27;
constexpr int B_HIGH_GPIO = 26;
constexpr int B_LOW_GPIO  = 25;
constexpr int C_HIGH_GPIO = 33;
constexpr int C_LOW_GPIO  = 32;

bool init_phases(esp_timer_handle_t *sine_generator_timer_handler);
void set_amplitude(float amplitude);

void IRAM_ATTR phase_output_intr(void* args);

inline uint32_t sin_lut(float) {
	return 0;
}
