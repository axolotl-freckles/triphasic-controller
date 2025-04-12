import numpy as np
LUT_RESOLUTION_bit = 5
LUT_RESOLUTION     = 2**LUT_RESOLUTION_bit
PWM_RESOLUTION = 15
MAX_DUTYCYCLE = 2**PWM_RESOLUTION - 1

sine_samples = np.zeros(LUT_RESOLUTION)
i = 0
x = 0
while x < 2*np.pi:
	sine_samples[i] = (0.5 + np.sin(x)/2) * MAX_DUTYCYCLE
	x += np.pi/16
	i += 1

print("#pragma once")
print()
print("#include <cstdint>")
print()
print(f"constexpr int      PWM_RESOLUTION   = {PWM_RESOLUTION:2d};")
print(f"constexpr uint32_t PWM_MAX_VAL = (1<<PWM_RESOLUTION) - 1;")
print()
print(f"constexpr int SINE_LUT_IDX_RESOLUTION_bit = {LUT_RESOLUTION_bit:2d};")
print(f"constexpr int SINE_LUT_IDX_RESOLUTION     = {LUT_RESOLUTION    :2d};")
print(f"constexpr int THETA_INT_RESOLUTION_bit    = 32;")
print()
print("inline uint8_t theta_int_to_lut_idx(uint32_t theta) {")
print("\treturn (uint8_t)(theta>>(THETA_INT_RESOLUTION_bit-SINE_LUT_IDX_RESOLUTION_bit));")
print("}")

print("const uint32_t SIN_LOOKUP[SINE_LUT_IDX_RESOLUTION] = {")
for i, sample in enumerate(sine_samples):
	sample_int = int(sample)
	sample_int += sample_int*0x10000
	print(f' 0x{int(sample_int):08X}', end=',' if i<31 else '')
	if i%8 == 7:
		print()
print("};")

print("inline uint32_t sin_lut(uint32_t x) {")
print("\tuint8_t lut_idx = theta_int_to_lut_idx(x);")
print("\treturn SIN_LOOKUP[lut_idx];")
print("}")
print()
