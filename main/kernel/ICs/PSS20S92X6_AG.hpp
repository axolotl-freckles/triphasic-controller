/**
 * @file PSS20S92X6_AG.hpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-07-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

namespace PSS20S92X6_AG {

static constexpr float MIN_Vot_READ = 1.13f;
static constexpr float MAX_Vot_READ = 2.77f;
static constexpr float MIN_TEMP_READ_C = 25.0f;
static constexpr float MAX_TEMP_READ_C = 90.0f;
static constexpr float Vot_RANGE    = MAX_Vot_READ - MIN_Vot_READ;
static constexpr float TEMP_RANGE_C = MAX_TEMP_READ_C - MIN_TEMP_READ_C;
static constexpr float V_to_TEMP_CONV = TEMP_RANGE_C/Vot_RANGE;
static constexpr float V_to_TEMP_INTR = MIN_TEMP_READ_C - MIN_Vot_READ*V_to_TEMP_CONV;

inline float voltage_to_temp_C(float V_ot) {
	return V_ot*V_to_TEMP_CONV + V_to_TEMP_INTR;
}

}