/**
 * @file ADS111x.hpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-04-09
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <stdint.h>

namespace ADS111x {

enum ADDRESS_POINTER : uint8_t {
	CONVERSION_REGISTER = 0b00,
	CONFIG_REGISTER     = 0b01,
	LO_THRESH_REGISTER  = 0b10,
	HI_THRESH_REGISTER  = 0b11
};

constexpr int8_t CONV_REG_SIZE_byte = 2;

// CONFIG REGISTER BITS ST:
constexpr int8_t OP_STATUS_bitn         = 15;
constexpr int8_t INPUT_MUX_CONF_hi_bitn = 14;
constexpr int8_t INPUT_MUX_CONF_lo_bitn = 12;
constexpr int8_t GAIN_AMP_CONF_hi_bitn  = 11;
constexpr int8_t GAIN_AMP_CONF_lo_bitn  =  9;
constexpr int8_t OP_MODE_bitn           =  8;
constexpr int8_t DATA_RATE_CONF_hi_bitn =  7;
constexpr int8_t DATA_RATE_CONF_lo_bitn =  5;
constexpr int8_t COMP_MODE_bitn         =  4;
constexpr int8_t COMP_POLARITY_bitn     =  3;
constexpr int8_t COMP_LATCH_bitn        =  2;
constexpr int8_t COMP_QUEUE_hi_bitn     =  1;
constexpr int8_t COMP_QUEUE_lo_bitn     =  0;
// :EN CONFIG REGISTER BITS

// CONFIG ENUMS ST:
constexpr uint16_t OP_STATUS_bit = 1<<OP_STATUS_bitn;
enum OP_STATUS_r : uint16_t {
	BUSY_CONV = 0b0<<OP_STATUS_bitn,
	FREE_CONV = 0b1<<OP_STATUS_bitn
};
enum OP_STATUS_w : uint16_t {
	START_CONV = 0b1<<OP_STATUS_bitn
};

enum INPUT_MUX : uint16_t {
	AINp_AIN0__AINn_AIN1 = 0b000<<INPUT_MUX_CONF_lo_bitn,
	AINp_AIN0__AINn_AIN3 = 0b001<<INPUT_MUX_CONF_lo_bitn,
	AINp_AIN1__AINn_AIN3 = 0b010<<INPUT_MUX_CONF_lo_bitn,
	AINp_AIN2__AINn_AIN3 = 0b011<<INPUT_MUX_CONF_lo_bitn,
	AINp_AIN0__AINn_GND  = 0b100<<INPUT_MUX_CONF_lo_bitn,
	AINp_AIN1__AINn_GND  = 0b101<<INPUT_MUX_CONF_lo_bitn,
	AINp_AIN2__AINn_GND  = 0b110<<INPUT_MUX_CONF_lo_bitn,
	AINp_AIN3__AINn_GND  = 0b111<<INPUT_MUX_CONF_lo_bitn
};

enum GAIN_AMP : uint16_t {
	FSR_6_144V = 0b000<<GAIN_AMP_CONF_lo_bitn,
	FSR_4_096V = 0b001<<GAIN_AMP_CONF_lo_bitn,
	FSR_2_048V = 0b010<<GAIN_AMP_CONF_lo_bitn,
	FSR_1_024V = 0b011<<GAIN_AMP_CONF_lo_bitn,
	FSR_0_512V = 0b100<<GAIN_AMP_CONF_lo_bitn,
	FSR_0_256V = 0b111<<GAIN_AMP_CONF_lo_bitn
};

constexpr uint16_t OP_MODE_bit = 1<<OP_MODE_bitn;
enum OP_MODE : uint16_t {
	CONTINUOUS_CONV = 0b0<<OP_MODE_bitn,
	ONE_SHOT        = 0b1<<OP_MODE_bitn
};

enum DATA_RATE : uint16_t {
	SPS_8   = 0b000<<DATA_RATE_CONF_lo_bitn,
	SPS_16  = 0b001<<DATA_RATE_CONF_lo_bitn,
	SPS_32  = 0b010<<DATA_RATE_CONF_lo_bitn,
	SPS_64  = 0b011<<DATA_RATE_CONF_lo_bitn,
	SPS_128 = 0b100<<DATA_RATE_CONF_lo_bitn,
	SPS_250 = 0b101<<DATA_RATE_CONF_lo_bitn,
	SPS_475 = 0b110<<DATA_RATE_CONF_lo_bitn,
	SPS_860 = 0b111<<DATA_RATE_CONF_lo_bitn
};

constexpr uint16_t COMP_MODE_bit = 1<<COMP_MODE_bitn;
enum COMP_MODE : uint16_t {
	TRADITIONAL = 0b0<<COMP_MODE_bitn,
	WINDOW      = 0b1<<COMP_MODE_bitn
};

constexpr uint16_t COMP_POLARITY_bit = 1<<COMP_POLARITY_bitn;
enum COMP_POLARITY : uint16_t {
	ACTIVE_LOW  = 0b0<<COMP_POLARITY_bitn,
	ACTIVE_HIGH = 0b1<<COMP_POLARITY_bitn
};

constexpr uint16_t COMP_LATCH_bit = 1<<COMP_LATCH_bitn;
enum COMP_LATCH : uint16_t {
	NONLATCHING = 0b0<<COMP_LATCH_bitn,
	LATCHING    = 0b1<<COMP_LATCH_bitn
};

enum COMP_QUEUE : uint16_t {
	ASSERT_AFTER_ONE  = 0b00<<COMP_QUEUE_lo_bitn,
	ASSERT_AFTER_TWO  = 0b01<<COMP_QUEUE_lo_bitn,
	ASSERT_AFTER_FOUR = 0b10<<COMP_QUEUE_lo_bitn,
	COMP_DISABLE__ALERT_RDY_HIGH_IMPEDANCE = 0b11<<COMP_QUEUE_lo_bitn
};
// :EN CONFIG ENUMS

constexpr uint16_t DEFAULT_CONFIG = 
	(uint16_t)GAIN_AMP     ::FSR_2_048V  |
	(uint16_t)OP_MODE      ::ONE_SHOT    |
	(uint16_t)DATA_RATE    ::SPS_128     |
	(uint16_t)COMP_MODE    ::TRADITIONAL |
	(uint16_t)COMP_POLARITY::ACTIVE_LOW  |
	(uint16_t)COMP_LATCH   ::NONLATCHING |
	(uint16_t)COMP_QUEUE   ::COMP_DISABLE__ALERT_RDY_HIGH_IMPEDANCE
;

inline void prepare_uint16_2_buff(const uint16_t reg_value, uint8_t* const buffer) {
	buffer[0] = (reg_value>>8)&0xFF;
	buffer[1] =  reg_value    &0xFF;
}
inline uint16_t read_from_buff_2_uint16(const uint8_t* const buffer) {
	return buffer[0]<<8 | buffer[1];
}

// DATASHEET PARITY ALIASES
constexpr int8_t COMP_POL_bitn = COMP_POLARITY_bitn;
constexpr int8_t COMP_LAT_bitn = COMP_LATCH_bitn;

constexpr uint16_t OS_bit       = OP_STATUS_bit;
constexpr uint16_t MODE_bit     = OP_MODE_bit;
constexpr uint16_t COMP_POL_bit = COMP_POLARITY_bit;
constexpr uint16_t COMP_LAT_bit = COMP_LATCH_bitn;

typedef OP_STATUS_r   OS_r;
typedef OP_STATUS_w   OS_w;
typedef INPUT_MUX     MUX;
typedef GAIN_AMP      PGA;
typedef OP_MODE       MODE;
typedef DATA_RATE     DR;
typedef COMP_POLARITY COMP_POL;
typedef COMP_LATCH    COMP_LAT;
typedef COMP_QUEUE    COMP_QUE;

}