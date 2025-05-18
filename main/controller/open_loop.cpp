/**
 * @file open_loop.cpp
 * @author ACMAX (aavaloscorrales@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-02-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "open_loop.hpp"

#include <stdio.h>
#include "esp_log.h"

static const char OPN_LOOP_TAG[] = "Open loop controller";

static constexpr int ADC_BITWIDTH = 11;

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


OpenLoop::OpenLoop() : Controller()
, adc_handle(nullptr), loop_count(0)
{
	loop_count_thresh = (uint64_t)(get_sample_frequency_hz() / 5);
}

void OpenLoop::setup() {
	if (adc_handle == nullptr) {
		adc_oneshot_unit_init_cfg_t adc_unit_config = {
			.unit_id  = ADC_UNIT_2,
			.clk_src  = ADC_RTC_CLK_SRC_DEFAULT,
			.ulp_mode = ADC_ULP_MODE_DISABLE
		};
		ESP_ERROR_CHECK_WITHOUT_ABORT(
			adc_oneshot_new_unit(&adc_unit_config, &adc_handle)
		);
	}

	adc_oneshot_chan_cfg_t adc_channel_config = {
		.atten   = ADC_ATTEN_DB_2_5,
		.bitwidth = (adc_bitwidth_t)ADC_BITWIDTH
	};
	ESP_ERROR_CHECK_WITHOUT_ABORT(
		adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_0, &adc_channel_config)
	);

	set_amplitude(1.0);
	ESP_LOGI(OPN_LOOP_TAG, "All set!");
	ESP_LOGI(OPN_LOOP_TAG, "Sample frecuency: %.2fHz", get_sample_frequency_hz());
}

void OpenLoop::loop() {
	int adc_read = 0;
	adc_oneshot_read(adc_handle, ADC_CHANNEL_0, &adc_read);

	float selected_hz = map_value(adc_read, 0, (1<<ADC_BITWIDTH)-1, 60.0f, 120.0f);
	if (++loop_count > loop_count_thresh) {
		loop_count = 0;
		(void)printf("\rSet frecuency: %6.2fHz", selected_hz);
	}

	set_frequency(selected_hz);
}
