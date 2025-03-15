import numpy as np
MAX_DUTYCYCLE = 255
sine_samples = np.zeros(32)
i = 0
x = 0
while x < 2*np.pi:
	sine_samples[i] = (0.5 + np.sin(x)/2) * MAX_DUTYCYCLE
	x += np.pi/16
	i += 1

print("const uint32_t SIN_LOOKUP[32] = {")
for i, sample in enumerate(sine_samples):
	sample_int = int(sample)
	sample_int += sample_int*0x10000
	print(f'\t0x{int(sample_int):08X}', end=',' if i<31 else '')
	if i%8 == 7:
		print()
print("};")
