/**
 * @file example_controllers.hpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-10-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <functional>

#include "controller/controller.hpp"
#include "time_series/num_calculus.hpp"

class PID : public Controller {
public:
	PID (std::function<float ()> error_function, float Kp, float Ki, float Kd);
	
	void setup() override;
	void loop()  override;

	struct ErrorFunction {
		float _setpoint;
		std::function<float ()> _selected_sensor_reading;
		enum SensorSource {
			CURRENT,
			SOURCE_VOLTAGE,
			PHASE_VOLTAGE
		};

		float operator() (void) const;

		ErrorFunction (float setpoint, SensorSource source);
	};

private:
	Integrator _integrator;
	Derivator  _derivator;
	std::function<float ()> _error_function;

	float _kp;
	float _ki;
	float _kd;
};

/* IMPLEMENTATIONS */

PID::PID (std::function<float ()> error_function, float Kp, float Ki, float Kd)
: Controller()
	, _integrator(get_sample_time_s())
	, _derivator(get_sample_time_s())
	, _error_function(error_function)
	, _kp(Kp)
	, _ki(Ki)
	, _kd(Kd)
{}

void PID::setup() {
	_integrator.setIntegralAcumulator(0.0f);
	set_amplitude(1.0);
}
void PID::loop() {
	float error = _error_function();
	float u = _kp*error + _kd*_derivator(error) + _ki*_integrator(error);
	set_frequency(u);
}

PID::ErrorFunction::ErrorFunction (float setpoint, SensorSource source)
:
	_setpoint(setpoint)
{
	switch (source) {
		case CURRENT:
			_selected_sensor_reading = Controller::read_pcb_current;
			break;
		case SOURCE_VOLTAGE: 
			_selected_sensor_reading = Controller::read_source_voltage;
			break;
		case PHASE_VOLTAGE:
			_selected_sensor_reading = Controller::read_phase_voltage;
			break;
	}
}
float PID::ErrorFunction::operator() (void) const {
	return _setpoint - _selected_sensor_reading();
}