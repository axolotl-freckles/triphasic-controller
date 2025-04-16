import numpy as np
LUT_RESOLUTION_bit = 6
LUT_RESOLUTION     = 2**LUT_RESOLUTION_bit
PWM_RESOLUTION = 15
MAX_DUTYCYCLE = 2**PWM_RESOLUTION - 1

sine_samples = np.zeros(LUT_RESOLUTION)
i = 0
x = 0
while i < LUT_RESOLUTION:
	sine_samples[i] = np.sin(x)*MAX_DUTYCYCLE
	x += 2*np.pi/LUT_RESOLUTION
	i += 1

with open('./main/kernel/sine_LUT.hpp', 'w+') as lut_file:
	lut_file.write("#pragma once\n\n")
	lut_file.write("#include <cstdint>\n\n")

	lut_file.write(f"constexpr int      PWM_RESOLUTION   = {PWM_RESOLUTION:2d};\n")
	lut_file.write(f"constexpr uint32_t PWM_MAX_VAL = (1<<PWM_RESOLUTION) - 1;\n\n")

	lut_file.write(f"constexpr int SINE_LUT_IDX_RESOLUTION_bit = {LUT_RESOLUTION_bit:2d};\n")
	lut_file.write(f"constexpr int SINE_LUT_IDX_RESOLUTION     = {LUT_RESOLUTION    :2d};\n")
	lut_file.write(f"constexpr int THETA_INT_RESOLUTION_bit    = 32;\n\n")

	lut_file.write("inline uint8_t theta_int_to_lut_idx(uint32_t theta) {\n")
	lut_file.write("\treturn (uint8_t)(theta>>(THETA_INT_RESOLUTION_bit-SINE_LUT_IDX_RESOLUTION_bit));\n")
	lut_file.write("}\n")

	lut_file.write("const uint32_t SIN_LOOKUP[SINE_LUT_IDX_RESOLUTION] = {\n")
	for i, sample in enumerate(sine_samples):
		if sample >= 0.0:
			sample_int = int(sample)
		else:
			sample_int = int(abs(sample))*0x10000
			# sample_int += sample_int*0x10000
		lut_file.write('\t' if i%8 == 0 else ' ')
		lut_file.write(f'0x{int(sample_int):08X}{',' if i<LUT_RESOLUTION-1 else ''}')
		if i%8 == 7:
			lut_file.write("\n")
	lut_file.write("};\n")

	lut_file.write("inline uint32_t sin_lut(uint32_t x) {\n")
	lut_file.write("\tuint8_t lut_idx = theta_int_to_lut_idx(x);\n")
	lut_file.write("\treturn SIN_LOOKUP[lut_idx];\n")
	lut_file.write("}\n")
