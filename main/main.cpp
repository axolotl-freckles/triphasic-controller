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
#include "example_controllers.hpp"

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
		set_frequency(60.0f);
		ESP_LOGI(OPN_LOOP_TAG, "All set!");
		ESP_LOGI(OPN_LOOP_TAG, "Sample frecuency: %.2fHz", get_sample_frequency_hz());
	}

	void loop() override {
		float current_read[4];
		float voltage_read[4];
		for (uint8_t i=0; i<3; i++) {
				PhaseSelector phase = (PhaseSelector)i;
				current_read[i] = read_current(phase);
				voltage_read[i] = read_phase_voltage();
		}
		current_read[3] = read_pcb_current();
		voltage_read[3] = read_source_voltage();

		// float selected_hz = map_value(voltage_read[1], 0.0f, 3.3f, 60.0f, 120.0f);
		if (++loop_count > loop_count_thresh) {
			loop_count = 0;
			(void)printf("\rC:[%6.2f %6.2f %6.2f %6.2f]V:[%5.2f %5.2f %5.2f %5.2f]",
				current_read[0], current_read[1], current_read[2], current_read[3],
				voltage_read[0], voltage_read[1], voltage_read[2], voltage_read[3]
			);
			// (void)printf("\rSet frecuency: %6.2fHz", selected_hz);
		}

		// set_frequency(selected_hz);
	}
};

extern "C" void app_main(void) {
	(void)printf("----MAIN----\n");
	init_kernel();

	OpenLoop openLoopController;
	PID pidController(
		PID::ErrorFunction(12, PID::ErrorFunction::CURRENT),
		0.2f, 0.01f, 0.1f
	);

	activate_controller(&openLoopController);

	while (true) {
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

#endif