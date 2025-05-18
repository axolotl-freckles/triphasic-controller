/**
 * @file open_loop.hpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-05-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include "controller.hpp"

#include "esp_adc/adc_oneshot.h"

class OpenLoop : public Controller {
private:
	adc_oneshot_unit_handle_t adc_handle;
	uint64_t loop_count_thresh;
	uint64_t loop_count;

public:
	OpenLoop();

	void setup() override;
	void loop()  override;
};
