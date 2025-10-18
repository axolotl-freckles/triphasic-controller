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
}
float phases::get_amplitude(void) {
	if ( !_active ) {
		return 0.0f;
	}
	return _amplitude;
}

void phases::set_frequency(const float frequency_hz) {
	_frequency_hz = frequency_hz;
}
void phases::set_angular_speed(const float angular_speed_rads) {
	_frequency_hz = angular_speed_rads / M_TAU;
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

#endif // MCK_PHASE_MODULE
