#ifndef ANOMALYDETECTOR_HPP
#define ANOMALYDETECTOR_HPP


#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

namespace SmartSensor{


class AnomalyDetector{


    private:
    tflite::MicroErrorReporter  error_reporter_;
    tflite::AllOpsResolver      resolver_;
    uint8_t                     tensor_arena_[10 * 1024];
    tflite::MicroInterpreter*   interpreter_;


    public:
    AnomalyDetector();
    float predict(const float* sensor_data, int data_length);


}; //AnomalyDetector
} //namespace SmartSensor





#endif //ANOMALYDETECTOR.HPP