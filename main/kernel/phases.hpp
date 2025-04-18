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

#include "sine_LUT.hpp"

constexpr float M_TAU = M_PI*2;
constexpr uint32_t MAX_THETA_INT = UINT32_MAX;
constexpr uint32_t SINE_WAVE_SAMPLE_TIMEus = CONFIG_SINE_WAVE_SAMPLE_TIMEus;
constexpr float    SINE_WAVE_SAMPLE_TIMEs  = SINE_WAVE_SAMPLE_TIMEus*1e-6f;
constexpr uint32_t DEAD_TIME_nsX100 = 200;
constexpr ledc_timer_t PWM_TIMER_ID = LEDC_TIMER_0;
constexpr uint32_t PWM_FREQUENCY_Hz = 2000;

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

bool init_phases(void);

void start_phases(void);
void stop_phases(void);
void kill_phases(void);
bool is_active_phases(void);

void set_amplitude(const float amplitude);
float get_amplitude(void);

void set_frequency(const float frequency_hz);
void set_angular_speed(const float angular_speed_rads);
float get_angular_speed(void);
float get_frequency(void);

uint32_t hz_to_delta_theta_int(float frequency_hz);
uint32_t w_to_delta_theta_int(float angular_speed_rads);
uint32_t rad_to_theta_int(float x);

void IRAM_ATTR phase_output_intr(void* args);
