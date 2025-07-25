/**
 * @file sensors.hpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-02-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"

#include "firmware_types.hpp"

namespace sensors {

constexpr int SENSOR_BUS_SCL_GPIO = 22;
constexpr int SENSOR_BUS_SDA_GPIO = 21;

// constexpr uint64_t ADC_TIMEOUT_ms = 10;
// constexpr uint64_t ADC_TIMEOUT_us = ADC_TIMEOUT_ms*1000;
constexpr uint64_t ADC_TIMEOUT_us = 1000;

constexpr i2c_port_num_t SENSOR_I2C_PORT = 0;
constexpr uint32_t I2C_SPEED_hz = 400000;
constexpr int I2C_TIMEOUT_ms = 13;

constexpr uint16_t ADC_CURRENT_ADDR = 0b1001000;
constexpr uint16_t ADC_VOLTAGE_ADDR = 0b1001001;

bool init_sensors(void);

enum ADS_channel : uint8_t {
	A0 = 0,
	A1 = 1,
	A2 = 2,
	A3 = 3
};
enum device_selector {
	ADC0 = 0,
	ADC1,
};

esp_err_t prepare_adc(device_selector adc, ADS_channel channel);
float read_adc_conv(device_selector adc);

float read_temperature(void);

}
