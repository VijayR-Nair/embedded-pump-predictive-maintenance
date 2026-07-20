#pragma once
#include "ISensorDriver.hpp"
#include "SensorPacket.hpp" // <-- CRITICAL: Double check this is here!
#include <fstream>
#include <string>
#include <optional>

namespace SmartSensor {

class CSVReader : public ISensorDriver {
public:
    explicit CSVReader(const std::string& file_path);
    ~CSVReader() override;

    CSVReader(const CSVReader&) = delete;
    CSVReader& operator=(const CSVReader&) = delete;

    bool begin() override;
    bool is_ready() const override { return file_stream_.is_open(); }
    std::optional<SensorPacket> next_frame() override;

private:
    std::string file_path_;
    std::ifstream file_stream_;
    bool has_header_ = true;
};

} // namespace SmartSensor