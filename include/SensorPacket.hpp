#pragma once
#include <array>

namespace SmartSensor {

constexpr size_t NUM_SENSORS = 51;
constexpr size_t TIMESTAMP_LEN = 20; // 19 characters + 1 null terminator

struct SensorPacket {
    char timestamp[TIMESTAMP_LEN]; // Must be raw char array, NOT std::string!
    std::array<float, NUM_SENSORS> sensor_readings;
    int machine_status; 
};

} // namespace SmartSensor