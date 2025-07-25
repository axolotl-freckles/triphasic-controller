/**
 * @file dials.cpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-07-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "dials.hpp"

#include <gpio.h>

using io::KNOB_CLK_PIN;
using io::KNOB_DT_PIN;
using io::KNOB_SW_PIN;
using io::ON_OFF_BTN_PIN;

bool io::init_dials() {
	esp_err_t err_code = ESP_OK;
	uint64_t disable_pins = 0
		| 1<<KNOB_CLK_PIN
		| 1<<KNOB_DT_PIN
		| 1<<KNOB_SW_PIN
	;
	gpio_config_t inputs_bulk_config = {
		.pin_bit_mask = disable_pins,
		.mode         = gpio_mode_t::GPIO_MODE_DISABLE,
		.pull_up_en   = gpio_pullup_t::GPIO_PULLUP_DISABLE,
		.pull_down_en = gpio_pulldown_t::GPIO_PULLDOWN_DISABLE
	};
	err_code = ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&inputs_bulk_config));
}