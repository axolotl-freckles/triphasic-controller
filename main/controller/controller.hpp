/**
 * @file controller.hpp
 * @author ACMAX (aavaloscorrales@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-02-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include "../kernel/firmware_types.hpp"
#include "controller_types.hpp"
#include "windup.hpp"

using control::FluxSpeed;
using control::FluxSpeed_t;

class Controller {
public:
	Controller();

/*
 * OVERLOAD THEESE TWO FUNCTIONS FOR YOUR CONTROLLER
 * setup() -> runs after windup, use to set the internal variables in your
 *          controller
 * loop()  -> runs every kernel tick. put your controller logic here
 */
	virtual void setup() = 0;
	virtual void loop()  = 0;

	void set_amplitude(float amplitude);
	void set_frequency(float frequency_hz);
	void set_flux_angular_speed(float w_rads);
	void set_windup(const Windup *windup);

	static float read_pcb_current(void);
	static float read_source_voltage(void);
	static float read_current(PhaseSelector phase);
	static float read_phase_voltage(void);

	static float read_frequency_hz(void);
	static float read_flux_angular_speed_rads(void);
	static float read_amplitude(void);

	static float get_sample_time_s(void);
	static float get_sample_frequency_hz(void);

	/*#### Kernel interface ####*/
	inline const control::ControlPoint get_control_point() {
		return control_point;
	}
	inline const Windup *get_windup() { return windup; }

private:
	control::ControlPoint control_point;
	const Windup *windup;
	// TODO: Add windown objects
};
