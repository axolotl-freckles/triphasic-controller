/**
 * @file mck_sensors.cpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-10-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "../../kernel/sensors.hpp"

#ifdef MCK_SENSOR_MODULE

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "mck_AC_motor.hpp"

static QueueHandle_t ADC1_q;
static QueueHandle_t ADC2_q;
static uint8_t ADC_prepare[2] = {0};

bool sensors::init_sensors(void) {
	bool mck_motor_init_ok = mck_motor::init_motor();

	ADC1_q = mck_motor::getADCqueue(sensors::device_selector::ADC0);
	ADC2_q = mck_motor::getADCqueue(sensors::device_selector::ADC1);
}

esp_err_t sensors::prepare_adc (
	  sensors::device_selector adc
	, sensors::ADS_channel channel
) {
	ADC_prepare[adc] = (uint8_t) channel;
	return ESP_OK;
}

float sensors::read_adc_conv(sensors::device_selector adc) {
	float adc_state[mck_motor::ADC_BUFF_COUNT];

	switch (adc) {
		case sensors::device_selector::ADC0:
			xQueuePeek(ADC1_q, adc_state, portMAX_DELAY);
			break;
		case sensors::device_selector::ADC1:
			xQueuePeek(ADC2_q, adc_state, portMAX_DELAY);
			break;
	}

	return adc_state[ ADC_prepare[adc] ];
}

#endif // MCK_SENSOR_MODULE
