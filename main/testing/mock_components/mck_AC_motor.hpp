/**
 * @file mck_AC_motor.hpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-10-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "../../kernel/sensors.hpp"

namespace mck_motor {

constexpr UBaseType_t ADC_BUFF_COUNT = 4;
constexpr UBaseType_t ADC_BUFF_LEN   = sizeof(float) * ADC_BUFF_COUNT;

struct ACMotorPlantState_t {
	float dx;
	float dy;
};

QueueHandle_t getADCqueue(sensors::device_selector adc);

bool init_motor(void);

void time_step(void);

}

#include "sdkconfig.h"

#ifdef CONFIG_USE_MOCK_INTERFACE
  #define MCK_MOTOR_MODULE
#endif // CONFIG_USE_MOCK_INTERFACE
