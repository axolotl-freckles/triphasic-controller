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
		float setpoint;
		std::function<float ()> selected_sensor_reading;
		enum SensorSource {
			CURRENT,
			SOURCE_VOLTAGE,
			PHASE_VOLTAGE
		};

		float operator() (void) const;

		ErrorFunction (float setpoint, SensorSource source);
	};

private:
	Integrator integrator;
	Derivator  derivator;
	std::function<float ()> error_function;

	float kp;
	float ki;
	float kd;
};

/* IMPLEMENTATIONS */

PID::PID (std::function<float ()> error_function, float Kp, float Ki, float Kd)
: Controller(),
	integrator(get_sample_time_s()),
	derivator(get_sample_time_s()),
	error_function(error_function),
	kp(Kp), ki(Ki), kd(Kd)
{}

void PID::setup() {
	set_amplitude(1.0);
}
void PID::loop() {
	float error = error_function();
	float u = kp*error + kd*derivator(error) + ki*integrator(error);
	set_frequency(u);
}

PID::ErrorFunction::ErrorFunction (float setpoint, SensorSource source)
: setpoint(setpoint)
{
	switch (source) {
		case CURRENT:
			selected_sensor_reading = Controller::read_pcb_current;
			break;
		case SOURCE_VOLTAGE: 
			selected_sensor_reading = Controller::read_source_voltage;
			break;
		case PHASE_VOLTAGE:
			selected_sensor_reading = Controller::read_phase_voltage;
			break;
	}
}
float PID::ErrorFunction::operator() (void) const {
	return setpoint - selected_sensor_reading();
}