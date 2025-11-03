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

Controller::Controller() : 
	  windup (nullptr)
	, winddown(nullptr)
{
	this->control_point.amplitude = 0.0f;
	this->control_point.flux_speed.type  = control::FluxSpeed_t::FREQUENCY;
	this->control_point.flux_speed.value = 0.0f;
}

void Controller::set_amplitude(float amplitude) {
	this->control_point.amplitude = amplitude;
}
void Controller::set_frequency(float frequency_hz) {
	this->control_point.flux_speed.value = frequency_hz;
	this->control_point.flux_speed.type  = control::FluxSpeed_t::FREQUENCY;
}
void Controller::set_flux_angular_speed(float w_rads) {
	this->control_point.flux_speed.value = w_rads;
	this->control_point.flux_speed.type  = control::FluxSpeed_t::ANGULAR_SPEED;
}
void Controller::set_windup  (const Windup   *windup  ) {
	this->windup   = windup;
}
void Controller::set_winddown(const Winddown *winddown) {
	this->winddown = winddown;
}

float Controller::read_pcb_current(void) {
	return kernel::get_current();
}
float Controller::read_source_voltage(void) {
	return kernel::get_source_voltage();
}
float Controller::read_current(PhaseSelector phase) {
	return kernel::get_current(phase);
}
float Controller::read_phase_voltage(void) {
	return kernel::get_phase_voltage();
}

float Controller::read_frequency_hz(void) {
	return kernel::get_frequency();
}
float Controller::read_flux_angular_speed_rads(void) {
	return kernel::get_flux_angular_speed();
}
float Controller::read_amplitude(void) {
	return kernel::get_amplitude();
}

float Controller::get_sample_time_s(void) {
	return FIRMWARE_TICK_INTERVAL_ms * 1e-3;
}
float Controller::get_sample_frequency_hz(void) {
	return 1.0 / (FIRMWARE_TICK_INTERVAL_ms * 1e-3);
}