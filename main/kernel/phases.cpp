/**
 * @file phases.cpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-02-21
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "phases.hpp"

#include <algorithm>

#include "pwm.h"

constexpr int DUTYCYCLE_OFFSET = 16;
constexpr uint32_t DUTYCYCLE_MASK_LOW  = 0xFFFF;
constexpr uint32_t DUTYCYCLE_MASK_HIGH = DUTYCYCLE_MASK_LOW<<DUTYCYCLE_OFFSET;

enum PhaseSelector {A, B, C};
inline void set_phase_dutycycle(PhaseSelector phase, uint32_t value);

void phase_output_intr(void* args) {
	static float A_theta = 0;
	float angular_speed = 0;
	uint32_t amplitude = 0;
	//read angular_speed
	//read amplitude
	
	A_theta += angular_speed*SINE_WAVE_SAMPLE_TIMEs;
	if (A_theta > M_TAU) A_theta -= M_TAU;
	
	uint32_t A_dutycycle = sin_lut(A_theta        );
	uint32_t B_dutycycle = sin_lut(A_theta+M_TAU/3);
	uint32_t C_dutycycle = sin_lut(A_theta-M_TAU/3);

	set_phase_dutycycle(A, A_dutycycle);
	set_phase_dutycycle(B, B_dutycycle);
	set_phase_dutycycle(C, C_dutycycle);
}

inline void set_phase_dutycycle(PhaseSelector phase, uint32_t value) {
	ledc_channel_t phase_component_h = A_HIGH_CHANNEL;
	ledc_channel_t phase_component_l = A_LOW_CHANNEL;
	switch (phase) {
		case A:
			phase_component_h = A_HIGH_CHANNEL;
			phase_component_l = A_LOW_CHANNEL;
			break;
		case B:
			phase_component_h = B_HIGH_CHANNEL;
			phase_component_l = B_LOW_CHANNEL;
			break;
		case C:
			phase_component_h = C_HIGH_CHANNEL;
			phase_component_l = C_LOW_CHANNEL;
			break;
	}
	pwm_set_duty(phase_component_h, (value&DUTYCYCLE_MASK_HIGH)>>DUTYCYCLE_OFFSET);
	pwm_set_duty(phase_component_l, value&DUTYCYCLE_MASK_LOW);
}

void set_amplitude(float amplitude) {
	float pwm_dcy_equiv = std::clamp(amplitude, 0.0f, 1.0f)*PWM_MAX_VAL;
	pwm_set_duty(AMPLITUDE_PWM_CHANNEL, (uint32_t)pwm_dcy_equiv);
}

bool init_phases(esp_timer_handle_t *sine_generator_timer_handler) {
	esp_timer_create_args_t sine_generator_timer_cfg {
		.callback              = phase_output_intr,
		.arg                   = (void*)nullptr,
		.dispatch_method       = ESP_TIMER_TASK,
		.name                  = "SINE GEN",
		.skip_unhandled_events = false
	};
	ESP_ERROR_CHECK( esp_timer_create(
		&sine_generator_timer_cfg,
		sine_generator_timer_handler
	));

	ledc_timer_config_t pwm_timer_config = {
		.speed_mode      = LEDC_HIGH_SPEED_MODE,
		.duty_resolution = (ledc_timer_bit_t)PWM_RESOLUTION,
		.timer_num       = PWM_TIMER_SRC,
		.freq_hz         = 20000,
		.clk_cfg         = LEDC_AUTO_CLK,
		.deconfigure     = false
	};
	ESP_ERROR_CHECK(ledc_timer_config(&pwm_timer_config));

	ledc_channel_config_t channel_base_config = {
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.intr_type  = LEDC_INTR_DISABLE,
		.timer_sel  = PWM_TIMER_SRC,
		.duty       = 0x0F,
		.hpoint     = 0,
		.flags = {.output_invert = 0}
	};

	// PHASE A
	ledc_channel_config_t channelA_high_config = channel_base_config;
	channelA_high_config.gpio_num = A_HIGH_GPIO;
	channelA_high_config.channel  = A_HIGH_CHANNEL;
	ledc_channel_config_t channelA_low_config = channel_base_config;
	channelA_low_config.gpio_num = A_LOW_GPIO;
	channelA_low_config.channel  = A_LOW_CHANNEL;
	ESP_ERROR_CHECK(ledc_channel_config(&channelA_high_config));
	ESP_ERROR_CHECK(ledc_channel_config(&channelA_low_config));

	// PHASE B
	ledc_channel_config_t channelB_high_config = channel_base_config;
	channelB_high_config.gpio_num = B_HIGH_GPIO;
	channelB_high_config.channel  = B_HIGH_CHANNEL;
	ledc_channel_config_t channelB_low_config = channel_base_config;
	channelB_low_config.gpio_num = B_LOW_GPIO;
	channelB_low_config.channel  = B_LOW_CHANNEL;
	ESP_ERROR_CHECK(ledc_channel_config(&channelB_high_config));
	ESP_ERROR_CHECK(ledc_channel_config(&channelB_low_config));
	
	// PHASE C
	ledc_channel_config_t channelC_high_config = channel_base_config;
	channelC_high_config.gpio_num = C_HIGH_GPIO;
	channelC_high_config.channel  = C_HIGH_CHANNEL;
	ledc_channel_config_t channelC_low_config = channel_base_config;
	channelC_low_config.gpio_num = C_LOW_GPIO;
	channelC_low_config.channel  = C_LOW_CHANNEL;
	ESP_ERROR_CHECK(ledc_channel_config(&channelC_high_config));
	ESP_ERROR_CHECK(ledc_channel_config(&channelC_low_config));

	return true;
}
