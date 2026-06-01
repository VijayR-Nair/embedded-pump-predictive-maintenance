#pragma once

#include <array>
#include <cmath>
#include <algorithm>

constexpr std::array<float, 51> SENSOR_MIN = {
    0.0000f, 0.0000f, 33.1597f, 31.6406f, 2.7980f,
    0.0000f, 0.0145f, 0.0000f, 0.0289f, 0.0000f,
    0.0000f, 0.0000f, 0.0000f, 0.0000f, 32.4096f,
    0.0000f, 0.0000f, 0.0000f, 0.0000f,
    0.0000f, 95.5277f, 0.0000f, 0.0000f, 0.0000f,
    0.0000f, 43.1548f, 0.0000f, 4.3193f, 0.6366f,
    0.0000f, 23.9583f, 0.2407f, 6.4606f, 54.8824f,
    0.0000f, 2.2610f, 0.0000f, 24.4792f, 19.2708f,
    23.4375f, 20.8333f, 22.1354f, 24.4792f, 25.7523f,
    26.3310f, 26.3310f, 27.1991f, 26.3310f, 26.6204f,
    27.4884f
};

constexpr std::array<float, 51> SENSOR_MAX = {
    2.5490f, 56.7274f, 56.0330f, 48.2205f, 800.0000f,
    99.9999f, 22.2512f, 23.5966f, 24.3490f, 25.0000f,
    76.1069f, 60.0000f, 45.0000f, 31.1876f, 500.0000f,
    739.7415f, 599.9999f, 4.8732f, 878.9179f,
    448.9079f, 1107.5260f, 594.0611f, 1227.5640f, 1000.0000f,
    839.5750f, 1214.4200f, 2000.0000f, 1841.1460f, 1466.2810f,
    1600.0000f, 1800.0000f, 1839.2110f, 1578.6000f, 425.5498f,
    694.4791f, 984.0607f, 174.9012f, 417.7083f, 547.9166f,
    512.7604f, 420.3125f, 374.2188f, 408.5937f, 1000.0000f,
    320.3125f, 370.3704f, 303.5301f, 561.6320f, 464.4097f,
    1000.0000f
};

inline float scale_sensor_minmax(float raw, std::size_t i) {
    const float min_v = SENSOR_MIN[i];
    const float max_v = SENSOR_MAX[i];

    // sensor_15 currently has NAN constants in the original file.
    // Fix that in Python if sensor_15 is actually part of the trained model input.
    if (std::isnan(raw) || std::isnan(min_v) || std::isnan(max_v)) {
        return 0.0f;
    }

    const float range = max_v - min_v;
    if (range == 0.0f) {
        return 0.0f;
    }

    const float scaled = (raw - min_v) / range;
    return std::clamp(scaled, 0.0f, 1.0f);
}
