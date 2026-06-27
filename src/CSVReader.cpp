#include "CSVReader.hpp"
#include <sstream>
#include <cstring>

namespace SmartSensor {

CSVReader::CSVReader(const std::string& file_path) {
    file_stream_.open(file_path);
}

CSVReader::~CSVReader() {
    if (file_stream_.is_open()) {
        file_stream_.close();
    }
}

std::optional<SensorPacket> CSVReader::next_frame() {
    if (!file_stream_.is_open() || file_stream_.peek() == EOF) {
        return std::nullopt; 
    }

    std::string line;
    if (has_header_) {
        std::getline(file_stream_, line); // Skip CSV Header row
        has_header_ = false;
    }

    if (!std::getline(file_stream_, line)) {
        return std::nullopt;
    }

    std::stringstream ss(line);
    
    // !!! MAKE SURE THIS LINE IS EXACTLY LIKE THIS !!!
    std::string token; 
    
    SensorPacket packet{};

    if (std::getline(ss, token, ',')) { /* skip */ }

    // 1. Timestamp
    if (std::getline(ss, token, ',')) {
        std::strncpy(packet.timestamp, token.c_str(), TIMESTAMP_LEN - 1);
        packet.timestamp[TIMESTAMP_LEN - 1] = '\0';
    }
    // 2. 51 Sensors (52 columns in CSV; col 15 = sensor_15 is always empty, skip it)
    for (size_t col = 0, i = 0; col < 52 && i < NUM_SENSORS; ++col) {
        if (!std::getline(ss, token, ',')) break;
        if (col == 15) continue;                      // skip sensor_15 (not in model)
        packet.sensor_readings[i++] = token.empty() ? 0.0f : std::stof(token);
    }

    // 3. Status
    if (std::getline(ss, token, ',')) {
        if (token == "NORMAL") packet.machine_status = 0;
        else if (token == "BROKEN") packet.machine_status = 1;
        else packet.machine_status = 2; // RECOVERING
    }

    return packet;
}

} // namespace SmartSensor