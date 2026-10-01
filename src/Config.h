#pragma once
// Knobs a grown-up might want to tweak.

#include <stdint.h>

constexpr uint8_t kScreenRotation = 3;      // 1 or 3: flip if the picture is upside down
constexpr uint8_t kBrightness = 110;        // 0-255; kept low-ish for bedtime eyes
constexpr uint8_t kVolume = 140;            // 0-255 buzzer volume
constexpr uint8_t kLullabyVolume = 70;      // quieter for the final goodnight tune
constexpr uint32_t kIdleSleepMs = 3 * 60 * 1000;  // sleep after 3 min without a press
