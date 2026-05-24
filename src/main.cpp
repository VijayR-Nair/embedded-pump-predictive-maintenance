#include "CSVReader.hpp"
#include <iostream>

int main() {
    // Relative path to your Kaggle CSV file
    SmartSensor::CSVReader reader("data/sensor.csv");

    if (!reader.is_open()) {
        std::cerr << "[ERROR] Place your Kaggle sensor.csv file in a 'data/' directory." << std::endl;
        return -1;
    }

    std::cout << "[INFO] Processing Kaggle Pump Data Stream..." << std::endl;

    size_t frame_count = 0;
    while (auto frame = reader.next_frame()) {
        frame_count++;
        
        // Output every 1000th line so your console doesn't slow down
        if (frame_count % 1000 == 0) {
            // Keep your local variables
            float sensor_00_val = frame->sensor_readings[0]; 
            float sensor_50_val = frame->sensor_readings[50];

            // Use BOTH variables here so the compiler is satisfied!
            std::cout << "Processed " << frame_count << " rows. "
                      << "Time: " << frame->timestamp 
                      << " | Sensor_00: " << sensor_00_val 
                      << " | Sensor_50: " << sensor_50_val << std::endl;
        }
    }

    std::cout << "[SUCCESS] Finished processing total of " << frame_count << " entries!" << std::endl;
    return 0;
}