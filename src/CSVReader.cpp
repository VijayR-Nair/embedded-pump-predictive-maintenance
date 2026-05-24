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

    // 1. Timestamp
    if (std::getline(ss, token, ',')) {
        // token.c_str() converts the std::string into a raw const char* pointer
        std::strncpy(packet.timestamp, token.c_str(), TIMESTAMP_LEN - 1);
        packet.timestamp[TIMESTAMP_LEN - 1] = '\0'; // Direct null-termination safety
    }

    // 2. 51 Sensors
    for (size_t i = 0; i < NUM_SENSORS; ++i) {
        if (std::getline(ss, token, ',')) {
            try {
                packet.sensor_readings[i] = token.empty() ? 0.0f : std::stof(token);
            } catch (...) {
                packet.sensor_readings[i] = 0.0f;
            }
        }
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