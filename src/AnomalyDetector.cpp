#include "AnomalyDetector.hpp"
#include "model_data.h"

namespace SmartSensor{

    AnomalyDetector::AnomalyDetector() {


    const tflite::Model* model =
        tflite::GetModel(autoencoder_full_int8_tflite);

    // ✅ Safety check — did the model load correctly?
    if (model == nullptr) {
        error_reporter_.Report("ERROR: Failed to load model!\n");
        return;  // stop constructor early
    }

    // also verify the schema version matches your TFLite Micro version
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        error_reporter_.Report("ERROR: Model schema version mismatch!\n");
        return;
    }

    static tflite::MicroInterpreter static_interpreter(
        model,
        resolver_,
        tensor_arena_,
        sizeof(tensor_arena_),
        &error_reporter_
    );
    interpreter_ = &static_interpreter;

    interpreter_->AllocateTensors();
}

float AnomalyDetector::predict(const float* sensor_data, int data_length) {

    TfLiteTensor* input = interpreter_->input(0);     // 1. Get a pointer to the model's input tensor
    TfLiteTensor* output = interpreter_->output(0); 

    //quantize float to int 8  before feeding the model
    float in_scale = input->params.scale;   
    int32_t in_zero_point = input->params.zero_point;


    for (int i = 0; i < data_length; i++) {  
        int32_t q = (int32_t)(sensor_data[i] / in_scale) + in_zero_point;
        q = q < -128 ? -128 : (q > 127 ? 127 : q);                                  //clamping to avoid overflow
        input->data.int8[i] = (int8_t)q; 

    }

    interpreter_->Invoke();                                                         // 3. Run the model

    //Dequantization
    float out_scale = output->params.scale;
    int32_t out_zero_point = output->params.zero_point;

    //compute reconsrtuction error

    float mse = 0.0f;
    for (int i = 0; i < data_length; i++) {
        float reconstructed = (output->data.int8[i] - out_zero_point) * out_scale;
        float diff          = sensor_data[i] - reconstructed;
        mse                += diff * diff;

    }
    return mse / data_length;
   
}

}