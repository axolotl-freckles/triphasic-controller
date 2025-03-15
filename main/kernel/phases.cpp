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

static const char LOG_TAG[] = "phases";

constexpr int DUTYCYCLE_OFFSET = 16;
constexpr uint32_t DUTYCYCLE_MASK_LOW  = 0xFFFF;
constexpr uint32_t DUTYCYCLE_MASK_HIGH = DUTYCYCLE_MASK_LOW<<DUTYCYCLE_OFFSET;
constexpr uint32_t MAX_INT32 = 0xFFFFFFFF;
constexpr uint32_t PLS_M_TAU_3_INT = MAX_INT32/3;
constexpr uint32_t MNS_M_TAU_3_INT = (~PLS_M_TAU_3_INT) + 1;

static esp_timer_handle_t sine_generator_timer_handle;

static float MAX_ANGULAR_SPEED_rads = 0.0f;
static float MAX_FREQUENCY_hz = 0.0f;
static uint32_t MAX_ANGULAR_SPEED_int = 0;
static volatile uint32_t _angular_speed_int = 0;
static volatile float    _amplitude = 0.0f;

enum PhaseSelector {A=0, B, C};
inline void set_phase_dutycycle(PhaseSelector phase, uint32_t value);

uint32_t hz_to_delta_theta_int(float frequency_hz) {
	return std::ceil(frequency_hz*SINE_WAVE_SAMPLE_TIMEs*MAX_INT32);
}
uint32_t w_to_delta_theta_int(float angular_speed_rads) {
	return std::ceil(angular_speed_rads*SINE_WAVE_SAMPLE_TIMEs*MAX_INT32/M_TAU);
}
uint32_t rad_to_theta_int(float x) {
	while (x > M_TAU) x -= M_TAU;
	while (x <  0.0f) x += M_TAU;
	return (uint32_t)(x*MAX_INT32/M_TAU);
}

void phase_output_intr(void* args) {
	static uint32_t A_theta = 0;
	uint32_t angular_speed = _angular_speed_int;

	A_theta += _angular_speed_int;

	uint32_t A_dutycycle = sin_lut(A_theta                );
	uint32_t B_dutycycle = sin_lut(A_theta+PLS_M_TAU_3_INT);
	uint32_t C_dutycycle = sin_lut(A_theta+MNS_M_TAU_3_INT);

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

void set_amplitude(const float amplitude) {
	if (amplitude > 1.0f || amplitude < 0.0f) {
		ESP_LOGE(LOG_TAG, "Invalid amplitude, out of range! Clipping");
	}
	_amplitude = std::clamp(amplitude, 0.0f, 1.0f);
	float pwm_dcy_equiv = std::clamp(amplitude, 0.0f, 1.0f)*PWM_MAX_VAL;
	pwm_set_duty(AMPLITUDE_PWM_CHANNEL, (uint32_t)pwm_dcy_equiv);
}
float get_amplitude(void) {
	return _amplitude;
}

void set_frequency(const float frequency_hz) {
	if (frequency_hz < 0.0f) {
		ESP_LOGE(LOG_TAG, "Invalid frequency, negative! Clipping");
		_angular_speed_int = 0;
	}
	if (frequency_hz > (MAX_ANGULAR_SPEED_rads/M_TAU)) {
		ESP_LOGE(LOG_TAG, "Invalid frequency, too high! Clipping");
		_angular_speed_int = MAX_ANGULAR_SPEED_int;
		return;
	}
	_angular_speed_int = hz_to_delta_theta_int(frequency_hz);
}
void set_angular_speed(const float angular_speed_rads) {
	if (angular_speed_rads < 0.0f) {
		ESP_LOGE(LOG_TAG, "Invalid angular speed, negative! Clipping");
		_angular_speed_int = 0;
		return;
	}
	if (angular_speed_rads > MAX_ANGULAR_SPEED_rads) {
		ESP_LOGE(LOG_TAG, "Invalid angular speed, too high! Clipping");
		_angular_speed_int = MAX_ANGULAR_SPEED_int;
		return;
	}
	_angular_speed_int = w_to_delta_theta_int(angular_speed_rads);
}
float get_angular_speed(void) {
	return M_TAU*_angular_speed_int/(SINE_WAVE_SAMPLE_TIMEs*MAX_INT32);
}
float get_frequency(void) {
	return _angular_speed_int/(SINE_WAVE_SAMPLE_TIMEs*MAX_INT32);
}

void start_phases(void) {
	esp_err_t error_code = ESP_OK;
	ESP_ERROR_CHECK_WITHOUT_ABORT( esp_timer_start_periodic(
		sine_generator_timer_handle, SINE_WAVE_SAMPLE_TIMEus
	));
	if (error_code != ESP_OK) {
		ESP_LOGE( LOG_TAG,
			"error starting phases: %s",
			esp_err_to_name(error_code)
		);
	}
}

void stop_phases(void) {
	esp_err_t error_code = ESP_OK;
	error_code = ESP_ERROR_CHECK_WITHOUT_ABORT(esp_timer_stop(sine_generator_timer_handle));
	if (error_code != ESP_OK) {
		ESP_LOGE( LOG_TAG,
			"error stopping phases: %s",
			esp_err_to_name(error_code)
		);
	}
}

void kill_phases(void) {
	esp_err_t error_code = ESP_OK;
	set_amplitude(0.0f);
	error_code = ESP_ERROR_CHECK_WITHOUT_ABORT(esp_timer_stop(sine_generator_timer_handle));
	if (error_code != ESP_OK) {
		ESP_LOGE( LOG_TAG,
			"error stopping phases: %s",
			esp_err_to_name(error_code)
		);
	}
	set_phase_dutycycle(A, 0);
	set_phase_dutycycle(B, 0);
	set_phase_dutycycle(C, 0);
}

bool is_active_phases(void) {
	return esp_timer_is_active(sine_generator_timer_handle);
}

static const char INIT_LOG_TAG[] = "phase_init";

static inline bool init_phase_channel(
	const PhaseSelector phase,
	const ledc_channel_config_t channel_base_config
) {
	ESP_LOGI(INIT_LOG_TAG, "Configuring phase %c...", 'A'+phase);
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

bool init_phases(void) {
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
		&sine_generator_timer_handle
	)); if (error_code != ESP_OK) return false;
	ESP_LOGI(INIT_LOG_TAG, "Sine sampler created!");

	MAX_ANGULAR_SPEED_rads = (M_TAU/SINE_WAVE_SAMPLE_TIMEs)*0.9;
	MAX_FREQUENCY_hz       = 0.9/SINE_WAVE_SAMPLE_TIMEs;
	MAX_ANGULAR_SPEED_int  = w_to_delta_theta_int(MAX_ANGULAR_SPEED_rads);
	ESP_LOGI(INIT_LOG_TAG, "Maximum angular speed: %.3erad/s", MAX_ANGULAR_SPEED_rads);
	ESP_LOGI(INIT_LOG_TAG, "Maximum frequency    : %.3eHz", MAX_FREQUENCY_hz);
	
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

	ESP_LOGI(INIT_LOG_TAG, "Configuring amplitude channel...");
	ledc_channel_config_t amplitude_channel_config = channel_base_config;
	amplitude_channel_config.gpio_num = AMPLITUDE_GPIO;
	amplitude_channel_config.channel  = AMPLITUDE_PWM_CHANNEL;
	error_code = ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_channel_config(&amplitude_channel_config));
	if (error_code != ESP_OK) {
		ESP_LOGE( INIT_LOG_TAG,
			"Error configuring amplitude channel, ERRCODE:\n%s",
			esp_err_to_name(error_code)
		);
		return false;
	}
	ESP_LOGI(INIT_LOG_TAG, "Amplitude channel configured!");

	ledc_fade_func_install(0);

	return true;
}
