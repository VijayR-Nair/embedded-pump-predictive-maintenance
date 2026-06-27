#include "AnomalyDetector.hpp"
#include "model_data.h"
#include <cstdio>

uint8_t SmartSensor::AnomalyDetector::tensor_arena_[60 * 1024];

namespace SmartSensor {

AnomalyDetector::AnomalyDetector() {

    resolver_.AddFullyConnected();
    resolver_.AddRelu();
    resolver_.AddQuantize();
    resolver_.AddDequantize();

    const tflite::Model* model =
        tflite::GetModel(autoencoder_full_int8_tflite);

    if (model == nullptr) {
        printf("ERROR: Failed to load model!\n");
        return;
    }

    if (model->version() != TFLITE_SCHEMA_VERSION) {
        printf("ERROR: Model schema version mismatch!\n");
        return;
    }

    static tflite::MicroInterpreter static_interpreter(
        model, resolver_, tensor_arena_, sizeof(tensor_arena_)
    );
    interpreter_ = &static_interpreter;

    if (interpreter_->AllocateTensors() != kTfLiteOk) {
        printf("ERROR: AllocateTensors() failed!\n");
        interpreter_ = nullptr;
        return;
    }
}

float AnomalyDetector::predict(const float* sensor_data, int data_length) {

    if (interpreter_ == nullptr) {
        printf("ERROR: Interpreter not initialized!\n");
        return -1.0f;
    }

    TfLiteTensor* input  = interpreter_->input(0);
    TfLiteTensor* output = interpreter_->output(0);

    const float   in_scale      = input->params.scale;
    const int32_t in_zero_point = input->params.zero_point;
    static bool once = true;
    if (once) { once=false;
        printf("[D] in scale=%.8f zp=%d  out scale=%.8f zp=%d\n",
               in_scale, in_zero_point, output->params.scale, output->params.zero_point);
        printf("[D] input->data.raw=%p  output->data.raw=%p\n",
               input->data.raw, output->data.raw);
        printf("[D] sensor_data (data[] in main) addr=%p\n", (void*)sensor_data);
        fflush(stdout);
    }

    for (int i = 0; i < data_length; i++) {
        int32_t q = (int32_t)(sensor_data[i] / in_scale) + in_zero_point;
        q = q < -128 ? -128 : (q > 127 ? 127 : q);
        input->data.int8[i] = (int8_t)q;
    }

    interpreter_->Invoke();

    const float   out_scale      = output->params.scale;
    const int32_t out_zero_point = output->params.zero_point;

    static bool once2 = true;
    if (once2) { once2=false;
        printf("[D] sensor_data[0..4]: %.4f %.4f %.4f %.4f %.4f\n",
               sensor_data[0], sensor_data[1], sensor_data[2], sensor_data[3], sensor_data[4]);
        printf("[D] out int8[0..4]: %d %d %d %d %d\n",
               output->data.int8[0], output->data.int8[1],
               output->data.int8[2], output->data.int8[3], output->data.int8[4]);
        float partial_mse = 0.0f;
        for (int i = 0; i < 5; i++) {
            float rec = (float)(output->data.int8[i] - out_zero_point) * out_scale;
            float d   = sensor_data[i] - rec;
            printf("[D] i=%d sensor=%.4f out_int8=%d rec=%.4f diff=%.4f diff^2=%.4f\n",
                   i, sensor_data[i], (int)output->data.int8[i], rec, d, d*d);
            partial_mse += d*d;
        }
        printf("[D] partial mse (first 5 of 51): %.4f\n", partial_mse / 5.0f);
        printf("[D] ALL sensor_data: ");
        for (int i = 0; i < data_length; i++) printf("%.3f ", sensor_data[i]);
        printf("\n[D] ALL out int8:    ");
        for (int i = 0; i < data_length; i++) printf("%d ", output->data.int8[i]);
        printf("\n");
        fflush(stdout);
    }

    float mse = 0.0f;
    for (int i = 0; i < data_length; i++) {
        float reconstructed = (float)(output->data.int8[i] - out_zero_point) * out_scale;
        float diff          = sensor_data[i] - reconstructed;
        mse                += diff * diff;
    }

    return mse / (float)data_length;
}

} // namespace SmartSensor
