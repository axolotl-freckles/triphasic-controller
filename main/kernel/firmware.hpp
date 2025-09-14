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

#include "esp_timer.h"
#include "freertos/FreeRtos.h"
#include "freertos/queue.h"

#include "firmware_types.hpp"
#include "../controller/controller.hpp"
#include "../controller/windup.hpp"

constexpr uint32_t FIRMWARE_TASK_STACK_DEPTH = 2176;

constexpr int   FIRMWARE_TICK_INTERVAL_ms  = CONFIG_FIRMWARE_TICK_INTERVAL;
constexpr float FIRMWARE_TICK_INTERVAL_s   = FIRMWARE_TICK_INTERVAL_ms / 1000.0f;
constexpr uint64_t SENSOR_SAMPLE_TIME_us   = CONFIG_SENSOR_SAMPLE_TIME*1000;

void init_kernel();

int load_controller(Controller *new_controller);
int activate_controller(Controller *new_controller);
int deactivate_controller();

namespace kernel {

void idle_loop();
void windup(TickType_t& previous_wake_time);
void controller_loop();
void windown(TickType_t& previous_wake_time);

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

const Windup *get_default_windup();

}
