/**
 * @file uart_interface.hpp
 * @author ACMAX (aavaloscorrales@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-11-22
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <cstdint>
#include <cstddef>

#include "driver/uart.h"

namespace uart {

constexpr size_t MAX_CUSTOM_MESSAGE_LEN_bytes = 6;

// ----------------------------------------------------------------------- ENUMS

enum command_type_t : uint8_t {
	SET_AMPLITUDE  = 0x89,
	SET_FREQUENCY  = 0x8A,
	SET_FLUX_SPEED = 0x8B,
	VALUE_REQUEST  = 0x8C,
	STATE_REQUEST  = 0x8D,
	SET_MODE_POLL  = 0x8E,
	SET_MODE_CONT  = 0x8F,
	I2C_MESSAGE    = 0x7A,
	CUSTOM_MESSAGE = 0x7B
};

enum message_type_t : uint8_t {
	I2C,
	CUSTOM
};

enum available_value_t : uint8_t {
	FREQUENCY       = 0x01,
	AMPLITUDE       = 0x02,
	PCB_VOLTAGE     = 0x10,
	PHASE_A_VOLTAGE = 0x11,
	PHASE_B_VOLTAGE = 0x12,
	PHASE_C_VOLTAGE = 0x13,
	PCB_CURRENT     = 0x20,
	PHASE_A_CURRENT = 0x21,
	PHASE_B_CURRENT = 0x22,
	PHASE_C_CURRENT = 0x23
};

// ----------------------------------------------------------------------- TYPES

struct control_command {
	command_type_t type;
	union { // .data
		float             setpoint;
		available_value_t requested_value;
	} data;
};

struct control_message {
	message_type_t type;
	size_t len;
	union { // .data
		uint8_t uint_data[MAX_CUSTOM_MESSAGE_LEN_bytes];
		char    char_data[MAX_CUSTOM_MESSAGE_LEN_bytes];
	} data;
};

// ------------------------------------------------------------------- FUNCTIONS

bool init_uart_interface(void);

esp_err_t send_value(available_value_t value_type, float value);

esp_err_t send_message(control_message &message);
esp_err_t send_message(uint8_t *data, size_t len, message_type_t type);
esp_err_t send_message(char    *data, size_t len, message_type_t type);

esp_err_t receive_control_command(
	control_command *out_command,
	TickType_t timeout=portMAX_DELAY
);

esp_err_t receive_message(
	control_message *out_message,
	TickType_t timeout=portMAX_DELAY
);

}
