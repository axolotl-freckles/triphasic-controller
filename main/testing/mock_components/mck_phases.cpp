/**
 * @file mck_phases.cpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-10-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "../../kernel/phases.hpp"

#ifdef MCK_PHASE_MODULE

#include "esp_log.h"

using phases::M_TAU;

static const char LOG_TAG[] = "mock phases";

static volatile float _amplitude    = 0.0f;
static volatile float _frequency_hz = 0.0f;
static volatile bool  _active  = false;
static volatile bool  _init_ok = false;

bool phases::init_phases(void) {
	_init_ok = true;
	return true;
}
bool phases::init_phases_ok(void) {
	return _init_ok;
}

void phases::start_phases(void) {
	if ( !_init_ok ) {
		return;
	}
	_active = true;
}
void phases::stop_phases(void) {
	_active = false;
}
void phases::kill_phases(void) {
	_active = false;
}
bool phases::is_active_phases(void) {
	return _active;
}

void phases::set_amplitude(const float amplitude) {
	if (amplitude > 1.0f || amplitude < 0.0f) {
		ESP_LOGE(LOG_TAG, "Invalid amplitude, out of range! Clipping");
	}

	_amplitude = std::min(1.0f, std::max(amplitude, 0.0f));
#ifdef CONFIG_VERBOSE_MODULES
	ESP_LOGI(LOG_TAG, "Amplitude set %.3f", _amplitude);
#endif // CONFIG_VERBOSE_MODULES
}
float phases::get_amplitude(void) {
	if ( !_active ) {
		return 0.0f;
	}
	return _amplitude;
}

void phases::set_frequency(const float frequency_hz) {
	_frequency_hz = frequency_hz;
#ifdef CONFIG_VERBOSE_MODULES
	ESP_LOGI(LOG_TAG, "Frequency set %.3e Hz", _frequency_hz);
#endif // CONFIG_VERBOSE_MODULES
}
void phases::set_angular_speed(const float angular_speed_rads) {
	_frequency_hz = angular_speed_rads / M_TAU;
#ifdef CONFIG_VERBOSE_MODULES
	ESP_LOGI(LOG_TAG, "Frequency set %.3e Hz", _frequency_hz);
#endif // CONFIG_VERBOSE_MODULES
}
float phases::get_frequency(void) {
	if ( !_active ) {
		return 0.0f;
	}
	return _frequency_hz;
}
float phases::get_angular_speed(void) {
	if (!_active ) {
		return 0.0f;
	}
	return _frequency_hz * M_TAU;
}

void phases::phase_output_intr(void *args) {
	return;
}

#endif // MCK_PHASE_MODULE
