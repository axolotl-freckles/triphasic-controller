/**
 * @file controller_types.hpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-08-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

namespace control {

enum FluxSpeed_t {
	FREQUENCY, ANGULAR_SPEED
};

struct FluxSpeed {
	float value;
	FluxSpeed_t type;
};

struct ControlPoint {
	float amplitude;
	FluxSpeed flux_speed;
};

}
