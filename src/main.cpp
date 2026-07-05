#include "CSVReader.hpp"
#include "CircularBuffer.hpp"
#include "AnomalyDetector.hpp"
#include "Preprocessor.hpp"
#include <iostream>

// 99th percentile reconstruction MSE on NORMAL data, calibrated for INT8 quantization
const float THRESHOLD = 4.455676f;

int main() {
    SmartSensor::CSVReader reader("data/sensor.csv");

    if (!reader.is_open()) {
        std::cerr << "[ERROR] Place your Kaggle sensor.csv in 'data/'" << std::endl;
        return -1;
    }

    SmartSensor::AnomalyDetector detector;
    SmartSensor::Preprocessor preprocessor;

    std::cout << "[INFO] Processing Kaggle Pump Data Stream..." << std::endl;

    SmartSensor::CircularBuffer<float, 10> sensor_00_window;

    size_t frame_count  = 0;
    size_t anomaly_count = 0;

    while (auto frame = reader.next_frame()) {
        frame_count++;

        sensor_00_window.push(frame->sensor_readings[0]);

        // ── Preprocess: impute + scale (same as Colab) ──
        float data[51];
        for (int i = 0; i < 51; i++) data[i] = frame->sensor_readings[i];

        if (frame_count == 1) {
            std::cout << "[DBG] Raw readings[13..17]: "
                      << frame->sensor_readings[13] << " "  // sensor_13
                      << frame->sensor_readings[14] << " "  // sensor_14
                      << frame->sensor_readings[15] << " "  // sensor_16 (post-skip)
                      << frame->sensor_readings[16] << " "  // sensor_17
                      << frame->sensor_readings[17] << "\n" // sensor_18
                      << "[DBG] (slot15=sensor_16, slot16=sensor_17, slot17=sensor_18)\n";
        }

        preprocessor.process(data, 51);

        if (frame_count == 1) {
            std::cout << "[DBG] Z-scored[0..9]:";
            for (int i = 0; i < 10; i++) std::cout << " " << data[i];
            std::cout << "\n[DBG] sensor_data[17]=" << data[17] << "\n";
        }

        // ── Run inference ──
        float mse = detector.predict(data, 51);

        if (frame_count <= 10) {
            std::cout << "[MSE] Frame " << frame_count << " | MSE: " << mse << "\n";
        }

        // ── Anomaly decision ──
        if (mse > THRESHOLD) {
            anomaly_count++;
            if (frame_count > 10)
                std::cout << "[ANOMALY] Frame " << frame_count
                          << " | MSE: " << mse << std::endl;
        }

        // ── Your existing print every 1000 rows ──
        if (frame_count % 1000 == 0) {
            std::cout << "Processed " << frame_count << " rows."
                      << " | Sensor_00: " << frame->sensor_readings[0]
                      << " | Rolling Avg: " << sensor_00_window.get_average()
                      << " | MSE: " << mse
                      << std::endl;
        }
    }

    std::cout << "[SUCCESS] Processed " << frame_count << " frames. "
              << "Anomalies detected: " << anomaly_count << std::endl;
    return 0;
}