/**
 * @file mck_AC_motor.cpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-10-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "mck_AC_motor.hpp"

#ifdef MCK_MOTOR_MODULE

#include "freertos/task.h"

#include "../../kernel/firmware.hpp"
#include "../../kernel/phases.hpp"

using mck_motor::ADC_BUFF_COUNT;
using mck_motor::ADC_BUFF_LEN;

extern QueueHandle_t ADC1_q = xQueueCreate(1, ADC_BUFF_LEN);
extern QueueHandle_t ADC2_q = xQueueCreate(1, ADC_BUFF_LEN);
static TaskHandle_t motor_task_h;
static bool has_init = false;

static volatile mck_motor::ACMotorPlantState_t _motor_state = {
	.dx = 0.0f,
	.dy = 0.0f
};

QueueHandle_t mck_motor::getADCqueue(sensors::device_selector adc) {
	switch (adc) {
		case sensors::device_selector::ADC0:
			return ADC1_q;
		case sensors::device_selector::ADC1:
			return ADC2_q;
	}
}

static void motor_task(void *motor_args) {
	TickType_t previous_wake_time = xTaskGetTickCount();

	while (true) {
		mck_motor::time_step();

		(void)xTaskDelayUntil(
			&previous_wake_time,
			FIRMWARE_TICK_INTERVAL_ms/portTICK_PERIOD_MS
		);
	}
}

bool mck_motor::init_motor(void) {
	if ( has_init ) {
		return true;
	}
	xTaskCreate(
		motor_task, "motor task",
		512,
		(void*) nullptr,
		3,
		&motor_task_h
	);

	has_init = true;
	return true;
}

void mck_motor::time_step(void) {
	float adc1_state[ADC_BUFF_COUNT] = {0};
	float adc2_state[ADC_BUFF_COUNT] = {0};
	
	float amplitude = phases::get_amplitude();
	float frequency = phases::get_frequency();

	_motor_state.dx = frequency;
	_motor_state.dy = amplitude;

	adc1_state[0] = frequency;
	adc1_state[1] = frequency / 3;
	adc1_state[2] = frequency / 3;
	adc1_state[3] = frequency / 3;

	adc2_state[0] = 120.0f;
	adc2_state[1] = 120.0f * amplitude;
	adc2_state[1] = 80.0f;
	adc2_state[1] = 80.0f;

	xQueueOverwrite(ADC1_q, adc1_state);
	xQueueOverwrite(ADC2_q, adc2_state);
}

#endif // MCK_MOTOR_MODULE
