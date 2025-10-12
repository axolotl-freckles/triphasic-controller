#pragma once

#include <stdint.h>

constexpr int      PWM_RESOLUTION   = 15;
constexpr uint32_t PWM_MAX_VAL = (1<<PWM_RESOLUTION) - 1;

constexpr int SINE_LUT_IDX_RESOLUTION_bit =  5;
constexpr int SINE_LUT_IDX_RESOLUTION     = 32;
constexpr int THETA_INT_RESOLUTION_bit    = 32;

inline uint8_t theta_int_to_lut_idx(uint32_t theta) {
	return (uint8_t)(theta>>(THETA_INT_RESOLUTION_bit-SINE_LUT_IDX_RESOLUTION_bit));
}
const uint32_t SIN_LOOKUP[SINE_LUT_IDX_RESOLUTION] = {
	0x3FFF3FFF, 0x4C7B4C7B, 0x587D587D, 0x638D638D, 0x6D406D40, 0x75357535, 0x7B1F7B1F, 0x7EC47EC4,
	0x7FFF7FFF, 0x7EC47EC4, 0x7B1F7B1F, 0x75357535, 0x6D406D40, 0x638D638D, 0x587D587D, 0x4C7B4C7B,
	0x3FFF3FFF, 0x33833383, 0x27812781, 0x1C711C71, 0x12BE12BE, 0x0AC90AC9, 0x04DF04DF, 0x013A013A,
	0x00000000, 0x013A013A, 0x04DF04DF, 0x0AC90AC9, 0x12BE12BE, 0x1C711C71, 0x27812781, 0x33833383
};
inline uint32_t sin_lut(uint32_t x) {
	uint8_t lut_idx = theta_int_to_lut_idx(x);
	return SIN_LOOKUP[lut_idx];
}
