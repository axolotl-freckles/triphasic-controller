/**
 * @file testing.hpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_timer.h"

#define TEST_TAG "TESTING"
#define OK_NOK(x)(x?OK_MESSAGE:NOK_MESSAGE)

const char  OK_MESSAGE[] = "OK!";
const char NOK_MESSAGE[] = "NOT OK";

constexpr float EPSILON = 1e-4f;

bool test_phases(void);
