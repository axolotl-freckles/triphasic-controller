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
constexpr uint32_t MAX_INT32    = 0xFFFFFFFF;
constexpr uint32_t SINE_WAVE_SAMPLE_TIMEus = CONFIG_SINE_WAVE_SAMPLE_TIMEus;
constexpr float    SINE_WAVE_SAMPLE_TIMEs  = SINE_WAVE_SAMPLE_TIMEus*1e-6f;
constexpr ledc_timer_t PWM_TIMER_ID = LEDC_TIMER_0;
constexpr uint32_t PWM_FREQUENCY_Hz = 2000;
constexpr int      PWM_RESOLUTION   = 15;
constexpr uint32_t PWM_MAX_VAL = (1<<PWM_RESOLUTION) - 1;

constexpr ledc_channel_t AMPLITUDE_PWM_CHANNEL = LEDC_CHANNEL_0;
constexpr ledc_channel_t A_HIGH_CHANNEL = LEDC_CHANNEL_1;
constexpr ledc_channel_t A_LOW_CHANNEL  = LEDC_CHANNEL_2;
constexpr ledc_channel_t B_HIGH_CHANNEL = LEDC_CHANNEL_3;
constexpr ledc_channel_t B_LOW_CHANNEL  = LEDC_CHANNEL_4;
constexpr ledc_channel_t C_HIGH_CHANNEL = LEDC_CHANNEL_5;
constexpr ledc_channel_t C_LOW_CHANNEL  = LEDC_CHANNEL_6;

constexpr int AMPLITUDE_GPIO = 15;
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

inline uint8_t theta_int_to_lut_idx(uint32_t theta) {
	return (uint8_t)(theta>>(32-5));
}
const uint32_t SIN_LOOKUP[32] = {
	0x3FFF3FFF, 0x4C7B4C7B, 0x587D587D, 0x638D638D, 0x6D406D40, 0x75357535, 0x7B1F7B1F, 0x7EC47EC4,
	0x7FFF7FFF, 0x7EC47EC4, 0x7B1F7B1F, 0x75357535, 0x6D406D40, 0x638D638D, 0x587D587D, 0x4C7B4C7B,
	0x3FFF3FFF, 0x33833383, 0x27812781, 0x1C711C71, 0x12BE12BE, 0x0AC90AC9, 0x04DF04DF, 0x013A013A,
	0x00000000, 0x013A013A, 0x04DF04DF, 0x0AC90AC9, 0x12BE12BE, 0x1C711C71, 0x27812781, 0x33833383
};
inline uint32_t sin_lut(uint32_t x) {
	uint8_t lut_idx = theta_int_to_lut_idx(x);
	return SIN_LOOKUP[lut_idx];
}
