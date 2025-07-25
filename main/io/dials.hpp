/**
 * @file dials.hpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-07-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

namespace io {

constexpr int KNOB_CLK_PIN = 36;
constexpr int KNOB_DT_PIN  = 39;
constexpr int KNOB_SW_PIN  = 34;
constexpr int ON_OFF_BTN_PIN = 35;

bool init_dials();

}