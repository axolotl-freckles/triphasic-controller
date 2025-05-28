/**
 * @file main.cpp
 * @author ACMAX (aavaloscorrales@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-02-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "sdkconfig.h"
#ifndef CONFIG_UNIT_TEST_MODE

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#define MAIN_TAG "MAIN"

#include "kernel/firmware.hpp"
#include "controller/controller.hpp"
// #include "controller/open_loop.hpp"

#include "time_series/num_calculus.hpp"

static float map_value(
	const float value,
	const float min_value, const float max_value,
	const float lower_bound, const float upper_bound
) {
	const float value_range = max_value - min_value;
	const float map_range   = upper_bound - lower_bound;
	const float conv_fact = map_range / value_range;

	return (value-min_value)*conv_fact + lower_bound;
}

class OpenLoop : public Controller {
private:
	static constexpr char OPN_LOOP_TAG[] = "Open loop controller";
	uint64_t loop_count_thresh;
	uint64_t loop_count;

public:
	OpenLoop() : Controller()
	, loop_count_thresh((uint64_t)(get_sample_frequency_hz() / 5))
	, loop_count(0)
	{}

	void setup() override {
		set_amplitude(1.0);
		ESP_LOGI(OPN_LOOP_TAG, "All set!");
		ESP_LOGI(OPN_LOOP_TAG, "Sample frecuency: %.2fHz", get_sample_frequency_hz());
	}

	void loop() override {
		float adc_read[3];
		for (uint8_t i=0; i<3; i++) {
				PhaseSelector phase = (PhaseSelector)i;
				adc_read[i] = read_voltage(phase);
		}

		float selected_hz = map_value(adc_read[1], 2.73, 2.74, 60.0f, 120.0f);
		if (++loop_count > loop_count_thresh) {
			loop_count = 0;
			(void)printf("\rRead[%11.4e %11.4e %11.4e]",
				adc_read[0], adc_read[1], adc_read[2]);
			// (void)printf("Set frecuency: %6.2fHz\n\n", selected_hz);
		}

		// set_frequency(selected_hz);
	}
};

class PID : public Controller {
private:
	Integrator integrator;
	Derivator  derivator;

	float kp = 0.2;
	float kd = 0.1;
	float ki = 0.01;

public:
	PID() : Controller(), integrator(get_sample_time_s()), derivator(get_sample_time_s())
	{}

	void setup() override {
		set_amplitude(1.0);
	}

	void loop() override {
		float voltage = get_voltage();
		float error = 16 - voltage;
		float u = kp*error + kd*derivator(error) + ki*integrator(error);
		set_frequency(u);
	}
};

extern "C" void app_main(void) {
	(void)printf("----MAIN----\n");
	init_kernel();

	OpenLoop openLoopController;
	PID pidController;

	activate_controller(&openLoopController);

	while (true) {
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

#endif