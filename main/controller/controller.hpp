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

class Controller {
public:
	virtual void setup() = 0;
	virtual void loop()  = 0;

	enum Phase {
		A = 0, B, C
	};

	static void  set_amplitude(float amplitude);
	static void  set_frequency(float frequency_hz);
	static void  set_flux_angular_speed(float w_rads);
	static float read_pcb_current(void);
	static float read_current(Controller::Phase phase);
	static float read_voltage(Controller::Phase phase);

	static float read_frequency_hz(void);
	static float read_flux_angular_speed_rads(void);
	static float read_amplitude(void);

	static float sample_time_s(void);
	static float sample_frequency_hz(void);
};
