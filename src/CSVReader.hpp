#pragma once
#include "SensorPacket.hpp" // <-- CRITICAL: Double check this is here!
#include <fstream>
#include <string>
#include <optional>

namespace SmartSensor {

class CSVReader {
public:
    explicit CSVReader(const std::string& file_path);
    ~CSVReader();

    CSVReader(const CSVReader&) = delete;
    CSVReader& operator=(const CSVReader&) = delete;

    std::optional<SensorPacket> next_frame();
    bool is_open() const noexcept { return file_stream_.is_open(); }

private:
    std::ifstream file_stream_;
    bool has_header_{true};
};

} // namespace SmartSensor