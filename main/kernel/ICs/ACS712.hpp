/**
 * @file ACS7112.hpp
 * @author ACMAX (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-05-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

namespace ACS712 {

constexpr float ACS_05B_SENS_VA = 0.185f;
constexpr float ACS_20A_SENS_VA = 0.100f;
constexpr float ACS_30A_SENS_VA = 0.066f;

constexpr float ACS_05B_SENS_AV = 1.0f/ACS_05B_SENS_VA;
constexpr float ACS_20A_SENS_AV = 1.0f/ACS_20A_SENS_VA;
constexpr float ACS_30A_SENS_AV = 1.0f/ACS_30A_SENS_VA;

constexpr float ACS_05B_OFFSET_V = 0.040f;
constexpr float ACS_20A_OFFSET_V = 0.030f;
constexpr float ACS_30A_OFFSET_V = 2.500f;

constexpr float ACS_05B_OFFSET_A = ACS_05B_OFFSET_V*ACS_05B_SENS_AV*100.0;
constexpr float ACS_20A_OFFSET_A = ACS_20A_OFFSET_V*ACS_20A_SENS_AV*100.0;
constexpr float ACS_30A_OFFSET_A = ACS_30A_OFFSET_V*ACS_30A_SENS_AV*100.0;
}
