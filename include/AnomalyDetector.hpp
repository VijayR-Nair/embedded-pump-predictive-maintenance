#ifndef ANOMALYDETECTOR_HPP
#define ANOMALYDETECTOR_HPP

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

namespace SmartSensor {

class AnomalyDetector {

    private:
    tflite::MicroMutableOpResolver<10>  resolver_;
    static uint8_t                      tensor_arena_[60 * 1024];
    tflite::MicroInterpreter*           interpreter_;    // ← type was missing

    public:
    AnomalyDetector();
    float predict(const float* sensor_data, int data_length);

}; // AnomalyDetector

} // namespace SmartSensor

#endif // ANOMALYDETECTOR_HPP