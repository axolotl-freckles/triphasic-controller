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

#include "freertos/FreeRTOS.h"
#include "driver/i2c_master.h"

constexpr int SENSOR_BUS_SCL_GPIO = 22;
constexpr int SENSOR_BUS_SDA_GPIO = 21;

constexpr i2c_port_num_t SENSOR_I2C_PORT = 0;
constexpr uint32_t I2C_SPEED = 100000;
constexpr uint16_t ADC_CURENT_ADDR = 0b1001000;

enum SensorPhaseSelector {
	A = 0,
	B,
	C
};

void IRAM_ATTR update_readings(void* argp);

bool init_sensors(void);

float read_pcb_current(void);
float read_pcb_voltage(void);
float read_current(SensorPhaseSelector sensor);
float read_voltage(SensorPhaseSelector sensor);

float read_temperature(void);
