#pragma once
#include "SensorPacket.hpp"
#include <optional>

namespace SmartSensor {

// HAL contract: any sensor source (CSV file, I2C bus, mock) implements this
// so AnomalyDetector's caller never knows which backend it's talking to.
class ISensorDriver {
public:
    virtual ~ISensorDriver() = default;

    // One-time setup (bus init, calibration, file open). Returns false on failure.
    virtual bool begin() = 0;

    // Cheap check callable any time after begin(); must not mutate state.
    virtual bool is_ready() const = 0;

    // Pulls the next reading. nullopt means "no data right now" (EOF, no
    // new sample yet, etc.) — callers should not treat it as fatal.
    virtual std::optional<SensorPacket> next_frame() = 0;
};

} // namespace SmartSensor
