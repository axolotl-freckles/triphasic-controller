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
