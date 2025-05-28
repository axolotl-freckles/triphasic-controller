/**
 * @file kernel.cpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-02-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "firmware.hpp"
#include "freertos/FreeRTOS.h"

#include "esp_log.h"

#include "phases.hpp"
#include "sensors.hpp"

#include "../controller/controller.hpp"
#include "../time_series/filters.hpp"

const char LOG_TAG[] = "controller_kernel";

constexpr uint32_t PHASE_INIT_TASK_STACK_DEPTH = 2160;
constexpr float SENSOR_SAMPLE_TIME_s = SENSOR_SAMPLE_TIME_us*1e-6;
static volatile float rc_sample_time_frac = 0.0f; //0.001f;

static volatile float cached_phase_voltage[3] = {0.0f};
static volatile float cached_source_voltage = 0.0f;

static volatile float cached_phase_current[3] = {0.f};
static volatile float cached_source_current = 0.0f;

static LowPassRC phase_voltage_filter[3] = {
	LowPassRC(rc_sample_time_frac*SENSOR_SAMPLE_TIME_s, SENSOR_SAMPLE_TIME_s),
	LowPassRC(rc_sample_time_frac*SENSOR_SAMPLE_TIME_s, SENSOR_SAMPLE_TIME_s),
	LowPassRC(rc_sample_time_frac*SENSOR_SAMPLE_TIME_s, SENSOR_SAMPLE_TIME_s)
};
static LowPassRC source_voltage_filter = LowPassRC(
	rc_sample_time_frac*SENSOR_SAMPLE_TIME_s,
	SENSOR_SAMPLE_TIME_s
);

static LowPassRC phase_current_filter[3] = {
	LowPassRC(rc_sample_time_frac*SENSOR_SAMPLE_TIME_s, SENSOR_SAMPLE_TIME_s),
	LowPassRC(rc_sample_time_frac*SENSOR_SAMPLE_TIME_s, SENSOR_SAMPLE_TIME_s),
	LowPassRC(rc_sample_time_frac*SENSOR_SAMPLE_TIME_s, SENSOR_SAMPLE_TIME_s)
};
static LowPassRC source_current_filter = LowPassRC(
	rc_sample_time_frac*SENSOR_SAMPLE_TIME_s,
	SENSOR_SAMPLE_TIME_s
);

static esp_timer_handle_t sensor_sampler_timer_handle;

static Controller *selected_controller = nullptr;

static TaskHandle_t firmware_task_h;
static StaticEventGroup_t firmware_event_group;
static volatile EventGroupHandle_t firmware_event_group_h;
constexpr EventBits_t PHASES_INIT_EXIT   = 0b001;
constexpr EventBits_t PHASES_INIT_FAILED = 0b100;
constexpr EventBits_t SENSORS_INIT       = 0b010;

static void update_sensor_readings(void *__argp);
static void firmware_task(void *__argp);

static void init_phases(void* phase_ok_argp) {
	bool phases_ok = phases::init_phases();
	ESP_LOGI(LOG_TAG, "Phases finished initializing");

	EventBits_t phases_status = PHASES_INIT_EXIT;
	if (!phases_ok) phases_status |= PHASES_INIT_FAILED;

	xEventGroupSetBits(firmware_event_group_h, phases_status);
	vTaskSuspend(NULL);
}

void init_kernel() {
	ESP_LOGI(LOG_TAG, "Initializing...");
	esp_err_t err_code = ESP_OK;
	BaseType_t coreID = xPortGetCoreID();
	firmware_event_group_h = xEventGroupCreateStatic(&firmware_event_group);
	EventBits_t firmware_events = 0;

	TaskHandle_t core2_phase_start_h;
	xTaskCreatePinnedToCore(
		init_phases, "Phase init",
		PHASE_INIT_TASK_STACK_DEPTH,
		(void*)nullptr,
		3,
		&core2_phase_start_h,
		(coreID == 0)? 1:0
	);
	firmware_events = xEventGroupWaitBits(
		firmware_event_group_h,
		PHASES_INIT_EXIT | PHASES_INIT_FAILED,
		pdFALSE, pdFALSE,
		portMAX_DELAY
	);
	ESP_LOGI(LOG_TAG, "phases initialized");
	vTaskDelete(core2_phase_start_h);
	if (firmware_events&PHASES_INIT_FAILED) {
		ESP_LOGE(LOG_TAG, "error in phases!!");
		return;
	}
	else {
		ESP_LOGI(LOG_TAG, "phases ok!");
	}
	ESP_ERROR_CHECK(ledc_fade_func_install(0));
	phases::phase_output_intr(nullptr);

	bool sensors_ok = sensors::init_sensors();
	if (sensors_ok) {
		ESP_LOGI(LOG_TAG, "sensors ok!");
	}
	else {
		ESP_LOGE(LOG_TAG, "error in sensors!");
		return;
	}

	esp_timer_create_args_t sensor_sampler_timer_cfg {
		.callback              = update_sensor_readings,
		.arg                   = (void*)nullptr,
		.dispatch_method       = ESP_TIMER_TASK,
		.name                  = "SENS SAMP",
		.skip_unhandled_events = true
	};
	err_code = ESP_ERROR_CHECK_WITHOUT_ABORT(esp_timer_create(
		&sensor_sampler_timer_cfg,
		&sensor_sampler_timer_handle
	));

	if (err_code != ESP_OK) {
		ESP_LOGE(LOG_TAG, "Error creating sampling timer");
		return;
	}

	err_code = ESP_ERROR_CHECK_WITHOUT_ABORT(
		esp_timer_start_periodic(sensor_sampler_timer_handle, SENSOR_SAMPLE_TIME_us)
	);

	xTaskCreatePinnedToCore(
		firmware_task,
		"Firmware kernel",
		FIRMWARE_TASK_STACK_DEPTH,
		(void*)nullptr,
		2,
		&firmware_task_h,
		coreID
	);
}

static void update_sensor_readings(void *__argp) {
	static uint8_t i = 0;

	float voltage_read = sensors::read_adc_conv(sensors::ADC_CURRENT);
	float current_read = voltage_read;// sensors::read_adc_conv(sensors::ADC_VOLTAGE);
	// TODO: convert from voltage to current reading

	if (i < 3) {
		cached_phase_voltage[i] = phase_voltage_filter[i](voltage_read);
		cached_phase_current[i] = phase_current_filter[i](current_read);
	}
	else {
		cached_source_voltage = voltage_read;
		cached_source_current = current_read;
	}

	// TODO: check for annomalies
	i = (i+1)%4;
	sensors::ADS_channel next_chan = (sensors::ADS_channel)i;
	sensors::prepare_adc(sensors::ADC_CURRENT, next_chan);
	// sensors::prepare_adc(sensors::ADC_VOLTAGE, next_chan);
}

void kernel_loop() {
	// Read sensors
	// Update values

	// Send signas
	if (selected_controller != nullptr) {
		selected_controller->loop();

		phases::set_amplitude(selected_controller->amplitude);
		switch (selected_controller->flux_speed.type) {
			case Controller::FluxSpeed_t::FREQUENCY:
				phases::set_frequency(selected_controller->flux_speed.value);
				break;
			case Controller::FluxSpeed_t::ANGULAR_SPEED:
				phases::set_angular_speed(selected_controller->flux_speed.value);
				break;
		}
	}
	else {
		if (phases::is_active_phases()) phases::stop_phases();
	}
}

static void firmware_task(void *__argp) {
	TickType_t previous_wake_time = xTaskGetTickCount();
	while (true) {
		kernel_loop();
		(void)xTaskDelayUntil(&previous_wake_time, FIRMWARE_TICK_INTERVAL_ms/portTICK_PERIOD_MS);
	}
}


int activate_controller(Controller *new_controller) {
	if (phases::is_active_phases()) {
		phases::stop_phases();
	}
	if (selected_controller != nullptr) {
		selected_controller = nullptr;
	}
	selected_controller = new_controller;
	new_controller->setup();
	phases::start_phases();
	return 0;
}
int deactivate_controller() {
	phases::stop_phases();
	selected_controller = nullptr;
	return 0;
}
void set_rc_mul_filter_value(float rc_mult) {
	rc_sample_time_frac = rc_mult;
	for (int i=0; i<3; i++) {
		phase_current_filter[i].set_rc(rc_mult*SENSOR_SAMPLE_TIME_s);
		phase_voltage_filter[i].set_rc(rc_mult*SENSOR_SAMPLE_TIME_s);
	}
	source_current_filter.set_rc(rc_mult*SENSOR_SAMPLE_TIME_s);
	source_voltage_filter.set_rc(rc_mult*SENSOR_SAMPLE_TIME_s);
}
float get_rc_mul(void) { return rc_sample_time_frac; }
float get_rc(void) { return rc_sample_time_frac*SENSOR_SAMPLE_TIME_s; }

float get_voltage(PhaseSelector phase) {
	return cached_phase_voltage[phase];
}
float get_voltage(void) {
	return cached_source_voltage;
}
float get_current(PhaseSelector phase) {
	return cached_phase_current[phase];
}
float get_current(void) {
	return cached_source_current;
}
float get_frequency(void) {
	return phases::get_frequency();
}
float get_flux_angular_speed(void) {
	return phases::get_angular_speed();
}
float get_amplitude(void) {
	return phases::get_amplitude();
}
