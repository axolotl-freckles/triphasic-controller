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

#include "esp_log.h"

#include "pwm.h"

constexpr int DUTYCYCLE_OFFSET = 16;
constexpr uint32_t DUTYCYCLE_MASK_LOW  = 0xFFFF;
constexpr uint32_t DUTYCYCLE_MASK_HIGH = DUTYCYCLE_MASK_LOW<<DUTYCYCLE_OFFSET;

enum PhaseSelector {A=0, B, C};
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

static const char INIT_LOG_TAG[] = "phase_init";

static inline bool init_phase_channel(
	const PhaseSelector phase,
	const ledc_channel_config_t channel_base_config
) {
	ESP_LOGI(INIT_LOG_TAG, "Configuring phase %c", 'A'+phase);
	ledc_channel_config_t channel_high_config = channel_base_config;
	ledc_channel_config_t channel_low_config  = channel_base_config;

	switch (phase) {
		case A:
			channel_high_config.gpio_num = A_HIGH_GPIO;
			channel_high_config.channel  = A_HIGH_CHANNEL;
			channel_low_config.gpio_num  = A_LOW_GPIO;
			channel_low_config.channel   = A_LOW_CHANNEL;
			break;
		case B:
			channel_high_config.gpio_num = B_HIGH_GPIO;
			channel_high_config.channel  = B_HIGH_CHANNEL;
			channel_low_config.gpio_num  = B_LOW_GPIO;
			channel_low_config.channel   = B_LOW_CHANNEL;
			break;
		case C:
			channel_high_config.gpio_num = C_HIGH_GPIO;
			channel_high_config.channel  = C_HIGH_CHANNEL;
			channel_low_config.gpio_num  = C_LOW_GPIO;
			channel_low_config.channel   = C_LOW_CHANNEL;
			break;
	}

	esp_err_t error_code = ESP_OK;
	error_code = ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_channel_config(&channel_high_config));
	if (error_code != ESP_OK) {
		ESP_LOGE( INIT_LOG_TAG,
			"Error configuring phase %c high, ERRCODE:\n%s",
			phase, esp_err_to_name(error_code));
		return false;
	}
	error_code = ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_channel_config(&channel_low_config));
	if (error_code != ESP_OK) {
		ESP_LOGE( INIT_LOG_TAG,
			"Error configuring phase %c low, ERRCODE:\n%s",
			phase, esp_err_to_name(error_code));
		return false;
	}
	ESP_LOGI(INIT_LOG_TAG, "Phase %c configured!", 'A'+phase);

	return true;
}

bool init_phases(esp_timer_handle_t *sine_generator_timer_handler) {
	ESP_LOGI(INIT_LOG_TAG, "Creating sine sampler...");
	esp_err_t error_code = ESP_OK;

	esp_timer_create_args_t sine_generator_timer_cfg {
		.callback              = phase_output_intr,
		.arg                   = (void*)nullptr,
		.dispatch_method       = ESP_TIMER_TASK,
		.name                  = "SINE GEN",
		.skip_unhandled_events = false
	};
	error_code = ESP_ERROR_CHECK_WITHOUT_ABORT(esp_timer_create(
		&sine_generator_timer_cfg,
		sine_generator_timer_handler
	)); if (error_code != ESP_OK) return false;
	ESP_LOGI(INIT_LOG_TAG, "Sine sampler created!");
	
	ESP_LOGI(INIT_LOG_TAG, "Configuring PWM timer...");
	ledc_timer_config_t pwm_timer_config = {
		.speed_mode      = LEDC_HIGH_SPEED_MODE,
		.duty_resolution = (ledc_timer_bit_t)PWM_RESOLUTION,
		.timer_num       = PWM_TIMER_ID,
		.freq_hz         = PWM_FREQUENCY_Hz,
		.clk_cfg         = LEDC_AUTO_CLK,
		.deconfigure     = false
	};
	error_code = ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_timer_config(&pwm_timer_config));
	if (error_code != ESP_OK) return false;
	ESP_LOGI(INIT_LOG_TAG, "PWM timer configured!");

	ledc_channel_config_t channel_base_config = {
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.intr_type  = LEDC_INTR_DISABLE,
		.timer_sel  = PWM_TIMER_ID,
		.duty       = 0x0F,
		.hpoint     = 0,
		.flags = {.output_invert = 0}
	};

	ESP_LOGI(INIT_LOG_TAG, "Configuring phase channels...");

	bool init_ok = false;
	init_ok = init_phase_channel(A, channel_base_config);
	if (!init_ok) return false;
	init_ok = init_phase_channel(B, channel_base_config);
	if (!init_ok) return false;
	init_ok = init_phase_channel(C, channel_base_config);
	if (!init_ok) return false;
	
	return true;
}
