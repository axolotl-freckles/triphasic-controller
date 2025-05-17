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
constexpr int ADC_ALERT_GPIO = 19;

// constexpr uint64_t ADC_TIMEOUT_ms = 10;
// constexpr uint64_t ADC_TIMEOUT_us = ADC_TIMEOUT_ms*1000;
constexpr uint64_t ADC_TIMEOUT_us = 1000;

constexpr i2c_port_num_t SENSOR_I2C_PORT = 0;
constexpr uint32_t I2C_SPEED_hz = 100000;
constexpr int I2C_TIMEOUT_ms = 13;

constexpr uint16_t ADC_CURENT_ADDR = 0b1001000;

void IRAM_ATTR update_readings(void* argp);

bool init_sensors(void);

float read_pcb_current(void);
float read_pcb_voltage(void);
float read_current(PhaseSelector sensor);
float read_voltage(PhaseSelector sensor);

float read_temperature(void);

}
