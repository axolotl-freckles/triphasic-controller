/**
 * @file kernel.hpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-02-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include "sdkconfig.h"

#include "esp_err.h"
#include "esp_timer.h"
#include "freertos/FreeRtos.h"
#include "freertos/queue.h"

#include "firmware_types.hpp"
#include "../controller/controller.hpp"
#include "../controller/windup.hpp"

constexpr uint32_t FIRMWARE_TASK_STACK_DEPTH = 2304;

constexpr int   FIRMWARE_TICK_INTERVAL_ms  = CONFIG_FIRMWARE_TICK_INTERVAL;
constexpr float FIRMWARE_TICK_INTERVAL_s   = FIRMWARE_TICK_INTERVAL_ms / 1000.0f;
constexpr uint64_t SENSOR_SAMPLE_TIME_us   = CONFIG_SENSOR_SAMPLE_TIME*1000;

void init_kernel();

int load_controller(Controller *new_controller);
int activate_controller(Controller *new_controller);
int deactivate_controller();

namespace kernel {

enum FirmwareState : EventBits_t {
	UNKNOWN      = 0,
	IDLE         = 0b00000001<<4,
	WINDUP       = 0b00000010<<4,
	CONTROL_LOOP = 0b00000100<<4,
	WINDDOWN     = 0b00001000<<4,
	ERROR        = 0b10000000<<4,
};

esp_err_t wait_until(FirmwareState state, TickType_t timeout = portMAX_DELAY);

inline esp_err_t wait_until_idle    (TickType_t timeout = portMAX_DELAY) {
	return wait_until(FirmwareState::IDLE, timeout);
}
inline esp_err_t wait_until_windup  (TickType_t timeout = portMAX_DELAY) {
	return wait_until(FirmwareState::WINDUP, timeout);
}
inline esp_err_t wait_until_winddown(TickType_t timeout = portMAX_DELAY) {
	return wait_until(FirmwareState::WINDDOWN, timeout);
}
inline esp_err_t wait_until_control_loop(TickType_t timeout = portMAX_DELAY) {
	return wait_until(FirmwareState::CONTROL_LOOP, timeout);
}

void idle_loop();
void windup(TickType_t& previous_wake_time);
void controller_loop();
void winddown(TickType_t& previous_wake_time);

void set_rc_mul_filter_value(float rc_mult);
float get_rc_mul(void);
float get_rc(void);

float get_phase_voltage(void);
float get_source_voltage(void);
float get_current(PhaseSelector phase);
float get_current(void);
float get_packet_temp(uint8_t packet_sel);

float get_frequency(void);
float get_flux_angular_speed(void);
float get_amplitude(void);

void set_default_windup_period(float period_s);
void set_default_windup_en_frequency(float frecuency_hz);

const Windup   *get_default_windup();
const Winddown *get_default_winddown();

} // namespace kernel
