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

#include <limits>
#include <functional>

#include "esp_log.h"

#include "ADS111x.hpp"

static const char INIT_LOG_TAG[] = "init_sensors";
static const char LOG_TAG[] = "sensors";

static i2c_master_bus_handle_t sensor_bus_h;

struct device_handle_t;
using ConfigFunction = std::function<bool (device_handle_t*)>;
static bool null_config_function(device_handle_t* dhp) {
	return true;
}
struct device_handle_t {
	const char* const name;
	i2c_master_dev_handle_t i2c_handle;
	bool ok;
	i2c_device_config_t config;
	ConfigFunction config_func;
};
enum device_selector {
	ADC_CURRENT = 0,
};

static device_handle_t i2c_devices[] = {
	{
		.name = "ADC CURRENT",
		.i2c_handle = nullptr,
		.ok = false,
		.config = {
			.dev_addr_length = I2C_ADDR_BIT_LEN_7,
			.device_address  = ADC_CURENT_ADDR,
			.scl_speed_hz    = I2C_SPEED,
			.scl_wait_us     = 0,
			.flags = {
				.disable_ack_check = true
			}
		},
		.config_func = [](device_handle_t* adc_current_hp) -> bool {
			esp_err_t error_code = ESP_OK;
			uint8_t tx_buff[3] = {0};
			tx_buff[0] = ADS111x::ADDRESS_POINTER::CONFIG_REGISTER;
			ADS111x::prepare_uint16_2_buff(ADS111x::DEFAULT_CONFIG, tx_buff+1);

			error_code = ESP_ERROR_CHECK_WITHOUT_ABORT(
				i2c_master_transmit(adc_current_hp->i2c_handle, tx_buff, 3, 10)
			);

			if (error_code != ESP_OK) return false;
			return true;
		}
	}
};

static constexpr int N_I2C_DEVICES = sizeof(i2c_devices)/sizeof(device_handle_t);

float read_current(SensorPhaseSelector sensor) {
	uint8_t tx_buff[3] = {0};
	uint8_t rx_buff[2] = {0};

	if (!i2c_devices[ADC_CURRENT].ok) {
		return std::numeric_limits<float>::signaling_NaN();
	}
	i2c_master_dev_handle_t adc_current_h = i2c_devices[ADC_CURRENT].i2c_handle;
	uint16_t config_reg = ADS111x::DEFAULT_CONFIG | (uint16_t)ADS111x::OS_w::START_CONV;

	switch (sensor) {
		case A:
			config_reg |= ADS111x::INPUT_MUX::AINp_AIN0__AINn_GND;
			break;
		default:
			break;
	}

	tx_buff[0] = ADS111x::ADDRESS_POINTER::CONFIG_REGISTER;
	ADS111x::prepare_uint16_2_buff(config_reg, tx_buff+1);
	ESP_ERROR_CHECK_WITHOUT_ABORT(
		i2c_master_transmit(adc_current_h, tx_buff, 3, 10)
	);

	tx_buff[0] = ADS111x::ADDRESS_POINTER::CONVERSION_REGISTER;

	ESP_ERROR_CHECK_WITHOUT_ABORT(
		i2c_master_transmit_receive(adc_current_h, tx_buff, 1, rx_buff, 2, 10)
	);

	uint16_t uadc_reading = ADS111x::read_from_buff_2_uint16(rx_buff);
	uadc_reading = ~uadc_reading + 1;
	float adc_reading = 4.096*uadc_reading/UINT16_MAX;

	return adc_reading;
}

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
	ESP_LOGI(LOG_TAG, "I2C sensor master bus configured!");

	for (int i=0; i<N_I2C_DEVICES; i++) {
		err_code = ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_bus_add_device(
			sensor_bus_h, &i2c_devices[i].config, &i2c_devices[i].i2c_handle
		));
		if (err_code != ESP_OK) {
			ESP_LOGE(INIT_LOG_TAG,
				"Error adding %s, ERRCODE: %s",
				i2c_devices[i].name,
				esp_err_to_name(err_code)
			);
			return false;
		}
	}
	ESP_LOGI(LOG_TAG, "Sensors added to I2C bus!");

	bool sensors_answering  = true;
	bool sensors_configured = true;
	for (int i=0; i<N_I2C_DEVICES; i++) {
		err_code = i2c_master_probe(
			sensor_bus_h,
			i2c_devices[i].config.device_address,
			10
		);
		if (err_code != ESP_OK) {
			ESP_LOGE(INIT_LOG_TAG, "%s not responding", i2c_devices[i].name);
			sensors_answering = false;
		}
		else {
			ESP_LOGI(INIT_LOG_TAG, "%s ok!", i2c_devices[i].name);

			if (i2c_devices[i].config_func(i2c_devices+i)) {
				i2c_devices[i].ok = true;
				ESP_LOGI(INIT_LOG_TAG, "%s configured", i2c_devices[i].name);
			}
			else {
				ESP_LOGE(INIT_LOG_TAG, "%s error in configuration", i2c_devices[i].name);
				sensors_configured = false;
			}
		}
	}

	return sensors_answering && sensors_configured;
}
