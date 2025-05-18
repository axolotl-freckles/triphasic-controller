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

class Controller {
public:
	enum FluxSpeed_t {
		FREQUENCY, ANGULAR_SPEED
	};
	Controller();

	virtual void setup() = 0;
	virtual void loop()  = 0;

	float amplitude;
	struct FluxSpeed {
		float value;
		FluxSpeed_t type;
	} flux_speed;

	void  set_amplitude(float amplitude);
	void  set_frequency(float frequency_hz);
	void  set_flux_angular_speed(float w_rads);
	static float read_pcb_current(void);
	static float read_source_voltage(void);
	static float read_current(PhaseSelector phase);
	static float read_voltage(PhaseSelector phase);

	static float read_frequency_hz(void);
	static float read_flux_angular_speed_rads(void);
	static float read_amplitude(void);

	static float get_sample_time_s(void);
	static float get_sample_frequency_hz(void);
};
