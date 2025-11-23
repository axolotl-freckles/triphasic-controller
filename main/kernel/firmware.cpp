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
using namespace kernel;

#include <cassert>

#include "freertos/FreeRTOS.h"

#include "esp_log.h"

#include "phases.hpp"
#include "sensors.hpp"
#include "ICs/ACS712.hpp"
#include "ICs/PSS20S92X6_AG.hpp"
#include "../io/dials.hpp"
#include "../io/uart_interface.hpp"

#include "../controller/controller_types.hpp"
#include "../controller/controller.hpp"
#include "../time_series/filters.hpp"

using kernel::FirmwareState;

const char LOG_TAG[] = "controller_kernel";

constexpr uint32_t PHASE_INIT_TASK_STACK_DEPTH = 2160;
constexpr float SENSOR_SAMPLE_TIME_s = SENSOR_SAMPLE_TIME_us*1e-6;
constexpr float ADC_SAMPLE_TIME_s = SENSOR_SAMPLE_TIME_s*4;
static volatile float rc_sample_time_frac = 0.001f;

static volatile float cached_ADC0_voltage[4] = {0.0f};
static volatile float cached_ADC1_voltage[4] = {0.f};

static LowPassRC ADC0_voltage_filter[4] = {
	LowPassRC(rc_sample_time_frac*ADC_SAMPLE_TIME_s, ADC_SAMPLE_TIME_s),
	LowPassRC(rc_sample_time_frac*ADC_SAMPLE_TIME_s, ADC_SAMPLE_TIME_s),
	LowPassRC(rc_sample_time_frac*ADC_SAMPLE_TIME_s, ADC_SAMPLE_TIME_s),
	LowPassRC(rc_sample_time_frac*ADC_SAMPLE_TIME_s, ADC_SAMPLE_TIME_s)
};

static LowPassRC ADC1_voltage_filter[4] = {
	LowPassRC(rc_sample_time_frac*ADC_SAMPLE_TIME_s, ADC_SAMPLE_TIME_s),
	LowPassRC(rc_sample_time_frac*ADC_SAMPLE_TIME_s, ADC_SAMPLE_TIME_s),
	LowPassRC(rc_sample_time_frac*ADC_SAMPLE_TIME_s, ADC_SAMPLE_TIME_s),
	LowPassRC(rc_sample_time_frac*ADC_SAMPLE_TIME_s, ADC_SAMPLE_TIME_s)
};

static esp_timer_handle_t sensor_sampler_timer_handle;

static Controller *selected_controller = nullptr;
static LinearWindup defaultWindup = LinearWindup(
	6.0f,
	0.75f, 60.0f,
	1.00f, 60.0f,
	control::FluxSpeed_t::FREQUENCY
);
static LinearWinddown defaultWinddown = LinearWinddown(
	6.0f,
	1.00f, 60.0f,
	0.75f, 60.0f,
	control::FluxSpeed_t::FREQUENCY
);

static TaskHandle_t firmware_task_h;
static StaticEventGroup_t firmware_event_group;
static volatile EventGroupHandle_t firmware_event_group_h;
constexpr EventBits_t PHASES_INIT_EXIT   = 0b001;
constexpr EventBits_t PHASES_INIT_FAILED = 0b100;
constexpr EventBits_t SENSORS_INIT       = 0b010;

constexpr EventBits_t STATE_MASK = 0xFF<<4;

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

	bool dials_ok = io::init_dials();
	if (!dials_ok) {
		ESP_LOGE(LOG_TAG, "error in io dials!");
		return;
	}
	xEventGroupSetBits(firmware_event_group_h, FirmwareState::IDLE);

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

	float ADC0_read = sensors::read_adc_conv(sensors::ADC0);
	float ADC1_read = sensors::read_adc_conv(sensors::ADC1);

	cached_ADC1_voltage[i] = ADC1_voltage_filter[i](ADC0_read);
	cached_ADC0_voltage[i] = ADC0_voltage_filter[i](ADC1_read);

	// TODO: check for annomalies
	i = (i+1)%4;
	sensors::ADS_channel next_chan = (sensors::ADS_channel)i;
	sensors::prepare_adc(sensors::ADC0, next_chan);
	sensors::prepare_adc(sensors::ADC1, next_chan);
}

/**
 * @brief Blocks tast until kernel arrives at state
 * 
 * @param state   Kernel state to wait for
 * @param timeout Time before timeout
 * @return ESP_OK if the wait was successfull.
 *         ESP_ERR_TIMEOUT if the wait exeded timeout value.
 *         ESP_FAIL if the kernel went into ERROR during the wait.
 */
esp_err_t kernel::wait_until(FirmwareState state, TickType_t timeout) {
	EventBits_t kernel_status = 0;
	EventBits_t wait_for = (EventBits_t)FirmwareState::ERROR
	                     | (EventBits_t)state;
	kernel_status = xEventGroupWaitBits(
		firmware_event_group_h,
		wait_for,
		pdFALSE, pdFALSE,
		timeout
	);
	if (kernel_status & FirmwareState::ERROR) {
		return ESP_FAIL;
	}
	if (! (kernel_status & state) ) {
		return ESP_ERR_TIMEOUT;
	}

	return ESP_OK;
}

static inline FirmwareState get_firmware_state(void) {
	EventBits_t curr_state = xEventGroupGetBits(firmware_event_group_h) & STATE_MASK;
	return (FirmwareState)curr_state;
}
static inline void update_firmware_state(FirmwareState new_state) {
	EventBits_t curr_state = xEventGroupGetBits(firmware_event_group_h) & STATE_MASK;
	xEventGroupClearBits(firmware_event_group_h, curr_state);
	xEventGroupSetBits(firmware_event_group_h, new_state);
}

static inline void apply_control_point(control::ControlPoint control_point) {
	phases::set_amplitude(control_point.amplitude);
	switch (control_point.flux_speed.type) {
		case control::FluxSpeed_t::FREQUENCY:
			phases::set_frequency(control_point.flux_speed.value);
			break;
		case control::FluxSpeed_t::ANGULAR_SPEED:
			phases::set_angular_speed(control_point.flux_speed.value);
			break;
	}
}
void kernel::idle_loop() {

}
void kernel::windup(TickType_t &previous_wake_time) {
	const Windup *controller_windup = &defaultWindup;
	if (selected_controller == nullptr) {
		ESP_LOGW(LOG_TAG, "Windup cancelled, no controller!");
		update_firmware_state(FirmwareState::IDLE);
		return;
	}
	if (selected_controller->get_windup() == nullptr) {
		ESP_LOGI(LOG_TAG, "No windup, executing default");
	}
	else {
		controller_windup = selected_controller->get_windup();
	}
	if (!controller_windup->assert_windup()) {
		ESP_LOGW(LOG_TAG, "Windup has invalid growth! Using default.");
		controller_windup = &defaultWindup;
	}

	phases::set_amplitude(0.0f);
	phases::set_angular_speed(0.0f);
	phases::start_phases();
	ESP_LOGI(LOG_TAG, "Starting windup!");
	float delta_t = 0.0f;
	const float period = controller_windup->period();
	while (delta_t <= period) {
		ControlPoint control_point = controller_windup->step(delta_t);
		apply_control_point(control_point);
		delta_t += FIRMWARE_TICK_INTERVAL_s;
		(void)xTaskDelayUntil(
			&previous_wake_time,
			FIRMWARE_TICK_INTERVAL_ms/portTICK_PERIOD_MS
		);
	}
	selected_controller->setup();
	update_firmware_state(FirmwareState::CONTROL_LOOP);
	ESP_LOGI(LOG_TAG, "Ending windup!");
}
void kernel::controller_loop() {
	// Send signas
	if (selected_controller != nullptr) {
		selected_controller->loop();

		apply_control_point(selected_controller->get_control_point());
	}
	else {
		if (phases::is_active_phases()) phases::stop_phases();
		update_firmware_state(FirmwareState::IDLE);
	}
}
void kernel::winddown(TickType_t& previous_wake_time) {
	const Winddown *controller_winddown = &defaultWinddown;
	assert(selected_controller != nullptr);
	if (selected_controller->get_winddown() == nullptr) {
		ESP_LOGI(LOG_TAG, "No winddown, executing default");
	}
	else {
		controller_winddown = selected_controller->get_winddown();
	}
	if (!controller_winddown->assert_winddown()) {
		ESP_LOGW(LOG_TAG, "Windup has invalid shrink! Using default.");
		controller_winddown = &defaultWinddown;
	}

	ESP_LOGI(LOG_TAG, "Starting winddown!");
	float delta_t = 0.0f;
	const float period = controller_winddown->period();
	while (delta_t <= period) {
		ControlPoint control_point = controller_winddown->step(delta_t);
		apply_control_point(control_point);
		delta_t += FIRMWARE_TICK_INTERVAL_s;
		(void)xTaskDelayUntil(
			&previous_wake_time,
			FIRMWARE_TICK_INTERVAL_ms/portTICK_PERIOD_MS
		);
	}
	update_firmware_state(FirmwareState::IDLE);
	phases::stop_phases();
	ESP_LOGI(LOG_TAG, "Ending winddown!");
}

static void firmware_task(void *__argp) {
	TickType_t previous_wake_time = xTaskGetTickCount();
	FirmwareState kernel_state = UNKNOWN;
	while (true) {
		kernel_state = get_firmware_state();
		switch (kernel_state) {
			case IDLE:
				break;
			case WINDUP:
				windup(previous_wake_time);
				break;
			case CONTROL_LOOP:
				controller_loop();
				break;
			case WINDDOWN:
				winddown(previous_wake_time);
				break;
			case ERROR:
			default:
				break;
		}
		(void)xTaskDelayUntil(
			&previous_wake_time,
			FIRMWARE_TICK_INTERVAL_ms/portTICK_PERIOD_MS
		);
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
	update_firmware_state(FirmwareState::WINDUP);
	return 0;
}
int deactivate_controller() {
	if (get_firmware_state() != FirmwareState::CONTROL_LOOP) {
		ESP_LOGW(LOG_TAG, "Controller not in controll loop!");
		return 1;
	}
	update_firmware_state(FirmwareState::WINDDOWN);
	return 0;
}


void kernel::set_rc_mul_filter_value(float rc_mult) {
	rc_sample_time_frac = rc_mult;
	for (int i=0; i<4; i++) {
		ADC1_voltage_filter[i].set_rc(rc_mult*SENSOR_SAMPLE_TIME_s);
		ADC0_voltage_filter[i].set_rc(rc_mult*SENSOR_SAMPLE_TIME_s);
	}
}
float kernel::get_rc_mul(void) { return rc_sample_time_frac; }
float kernel::get_rc(void) { return rc_sample_time_frac*SENSOR_SAMPLE_TIME_s; }

float kernel::get_phase_voltage(void) {
	return cached_ADC1_voltage[sensors::ADS_channel::A1];
}
float kernel::get_source_voltage(void) {
	return cached_ADC1_voltage[sensors::ADS_channel::A0];
}
float kernel::get_current(PhaseSelector phase) {
	float offset = cached_ADC0_voltage[phase+1] - ACS712::ACS_30A_OFFSET_V;
	return offset*ACS712::ACS_30A_SENS_AV;
}
float kernel::get_current(void) {
	float offset = cached_ADC0_voltage[sensors::A0] - ACS712::ACS_30A_OFFSET_V;
	return offset*ACS712::ACS_30A_SENS_AV;
}
float kernel::get_packet_temp(uint8_t packet_sel) {
	if (packet_sel >= 2) {
		return std::numeric_limits<float>::signaling_NaN();
	}
	return PSS20S92X6_AG::voltage_to_temp_C(
		cached_ADC1_voltage[sensors::A2 + packet_sel]
	);
}
float kernel::get_frequency(void) {
	return phases::get_frequency();
}
float kernel::get_flux_angular_speed(void) {
	return phases::get_angular_speed();
}
float kernel::get_amplitude(void) {
	return phases::get_amplitude();
}

void kernel::set_default_windup_period(float period_s) {
	defaultWindup.set_period(period_s);
}
void kernel::set_default_windup_en_frequency(float frecuency_hz) {
	defaultWindup.set_en_flux_speed(frecuency_hz);
}

const Windup   *kernel::get_default_windup() {
	return &defaultWindup;
}
const Winddown *kernel::get_default_winddown() {
	return &defaultWinddown;
}
