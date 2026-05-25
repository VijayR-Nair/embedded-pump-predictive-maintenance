#include "CSVReader.hpp"
#include "CircularBuffer.hpp"
#include <iostream>

int main() {
    SmartSensor::CSVReader reader("data/sensor.csv");

    if (!reader.is_open()) {
        std::cerr << "[ERROR] Place your Kaggle sensor.csv file in a 'data/' directory." << std::endl;
        return -1;
    }

    std::cout << "[INFO] Processing Kaggle Pump Data Stream..." << std::endl;

    // Create a rolling window buffer that holds the last 10 readings for Sensor 00
    SmartSensor::CircularBuffer<float, 10> sensor_00_window;

    size_t frame_count = 0;
    while (auto frame = reader.next_frame()) {
        frame_count++;
        
        // Feed the current sensor reading into our rolling window
        sensor_00_window.push(frame->sensor_readings[0]);

        // Output every 1000th line to check our math
        if (frame_count % 1000 == 0) {
            float current_val = frame->sensor_readings[0];
            float rolling_avg = sensor_00_window.get_average();

            std::cout << "Processed " << frame_count << " rows. "
                      << " | Raw Sensor_00: " << current_val 
                      << " | 10-Sample Rolling Avg: " << rolling_avg << std::endl;
        }
    }

    std::cout << "[SUCCESS] Finished processing total of " << frame_count << " entries!" << std::endl;
    return 0;
}