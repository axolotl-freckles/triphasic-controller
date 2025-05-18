/**
 * @file controller.cpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-05-16
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "controller.hpp"
#include "../kernel/firmware.hpp"

Controller::Controller() {
	this->amplitude = 0.0f;
	this->flux_speed.type  = FREQUENCY;
	this->flux_speed.value = 0.0f;
}

void Controller::set_amplitude(float amplitude) {
	this->amplitude = amplitude;
}
void Controller::set_frequency(float frequency_hz) {
	this->flux_speed.value = frequency_hz;
	this->flux_speed.type  = FREQUENCY;
}
void Controller::set_flux_angular_speed(float w_rads) {
	this->flux_speed.value = w_rads;
	this->flux_speed.type  = ANGULAR_SPEED;
}

float Controller::read_pcb_current(void) {
	return get_current();
}
float Controller::read_source_voltage(void) {
	return get_voltage();
}
float Controller::read_current(PhaseSelector phase) {
	return get_current(phase);
}
float Controller::read_voltage(PhaseSelector phase) {
	return get_voltage(phase);
}

float Controller::read_frequency_hz(void) {
	return get_frequency();
}
float Controller::read_flux_angular_speed_rads(void) {
	return get_flux_angular_speed();
}
float Controller::read_amplitude(void) {
	return get_amplitude();
}

float Controller::get_sample_time_s(void) {
	return FIRMWARE_TICK_INTERVAL_ms * 1e-3;
}
float Controller::get_sample_frequency_hz(void) {
	return 1.0 / (FIRMWARE_TICK_INTERVAL_ms * 1e-3);
}