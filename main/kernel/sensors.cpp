/**
 * @file sensors.cpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-03-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "sensors.hpp"

#include "esp_log.h"

static const char INIT_LOG_TAG[] = "init_sensors";
static const char LOG_TAG[] = "sensors";

static i2c_master_bus_handle_t sensor_bus_h;
static i2c_master_dev_handle_t adc_current_h;

bool init_sensors(void) {
	esp_err_t err_code = ESP_OK;
	i2c_master_bus_config_t sensor_bus_config = {
		.i2c_port   = SENSOR_I2C_PORT,
		.sda_io_num = (gpio_num_t)SENSOR_BUS_SDA_GPIO,
		.scl_io_num = (gpio_num_t)SENSOR_BUS_SCL_GPIO,
		.clk_source = I2C_CLK_SRC_DEFAULT,
		.glitch_ignore_cnt = 7,
		.intr_priority = 0,
		.flags = {
			.enable_internal_pullup = true
		}
	};

	err_code = ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_new_master_bus(
		&sensor_bus_config, &sensor_bus_h
	));
	if (err_code != ESP_OK) {
		ESP_LOGE(INIT_LOG_TAG,
			"Error configuring sensor i2c bus, ERRCODE: %s",
			esp_err_to_name(err_code)
		);
		return false;
	}

	i2c_device_config_t adc_current_config = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address  = ADC_CURENT_ADDR,
		.scl_speed_hz    = I2C_SPEED,
		.scl_wait_us     = 0,
		.flags = {
			.disable_ack_check = true
		}
	};
	err_code = ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_bus_add_device(
		sensor_bus_h, &adc_current_config, &adc_current_h
	));
	if (err_code != ESP_OK) {
		ESP_LOGE(INIT_LOG_TAG,
			"Error adding current sensor, ERRCODE: %s",
			esp_err_to_name(err_code)
		);
		return false;
	}
	err_code = i2c_master_probe(sensor_bus_h, ADC_CURENT_ADDR, 2);
	if (err_code != ESP_OK) {
		ESP_LOGE(INIT_LOG_TAG, "ADC CURRENT not responding");
	}
	else {
		ESP_LOGI(INIT_LOG_TAG, "ADC CURRENT ok!");
	}
}
