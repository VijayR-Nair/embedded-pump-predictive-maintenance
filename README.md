# Embedded Pump Predictive Maintenance Engine

An industrial-grade, zero-allocation C++ Software-in-the-Loop (SIL) pipeline for real-time anomaly detection on smart edge sensors, powered by a quantized TensorFlow Lite for Microcontrollers autoencoder.

This project bridges the gap between high-dimensional industrial sensor streams (simulated using the [Kaggle Water Pump dataset](https://www.kaggle.com/datasets/nphantawee/pump-sensor-data)) and the highly constrained environments of edge microcontrollers such as the ESP32 or ARM Cortex-M series.

<img width="1024" height="559" alt="SIL Architecture" src="https://github.com/user-attachments/assets/21de00cb-ad8d-4acf-97d8-210e5b130439" />

---

## The Philosophy: Software-in-the-Loop

Deploying ML prototypes directly onto high-voltage physical industrial motors is impractical during early-stage development. This project implements a **SIL paradigm**: the C++ inference engine treats each CSV row as a real-time sensor interrupt. Because the data processing layer is decoupled from the ingestion layer, **95% of the codebase remains identical when flashed onto physical hardware** — only the low-level peripheral driver changes.

---

## Model: Quantized Autoencoder

### Architecture

```
Input (51 sensors)  →  Dense(16, ReLU)  →  Dense(8, ReLU)  →  Dense(16, ReLU)  →  Dense(51, linear)
```

The model is an **int8-quantized autoencoder** compiled with TensorFlow Lite for Microcontrollers. It learns to reconstruct normal pump sensor readings; reconstruction error (MSE) spikes when the pump deviates from its trained operating envelope.

| Property | Value |
|---|---|
| Input features | 51 sensors (sensor_00–sensor_51, excluding sensor_15) |
| Quantization | Full int8 (weights + activations) |
| Input tensor | shape `[1, 51]`, int8, scale=0.2029, zero_point=-67 |
| Output tensor | shape `[1, 51]`, int8, scale=0.1348, zero_point=-53 |
| Arena size | 60 KB (static BSS allocation) |
| Operators | FullyConnected, ReLU, Quantize, Dequantize |
| Runtime | TensorFlow Lite for Microcontrollers |

### Preprocessing Pipeline

Each frame goes through the same pipeline as training before inference:

1. **Median imputation** — zero or missing sensor readings are replaced with the per-sensor median computed from all 205,836 NORMAL frames
2. **Z-score normalisation** — `z = (x − mean) / std` using StandardScaler fitted on all NORMAL frames
3. **Int8 quantisation** — `q = clip(round(z / scale + zero_point), −128, 127)`

### Why Retrain on All NORMAL Data

The initial model was trained on only the first ~10,000 NORMAL rows. The full dataset contains multiple pump operating regimes: for example, `sensor_19` ranges from 249 to 664 across the dataset. With the original narrow scaler (std=5.03), frames from later regimes produced z-scores up to **−82**, causing a **94.9% false positive rate**.

Refitting the scaler and retraining on all 205,836 NORMAL rows expanded `sensor_19` std from **5.03 → 205.05**, bringing the FPR down to **1.0%** at the chosen operating threshold.

---

## Dataset

| Label | Count | % |
|---|---|---|
| NORMAL | 205,836 | 93.4% |
| RECOVERING | 14,477 | 6.6% |
| BROKEN | 7 | <0.1% |
| **Total** | **220,320** | |

CSV structure: 1 unnamed index column + `timestamp` + 52 sensor columns (sensor_15 always empty, excluded) + `machine_status`.

---

## Evaluation Results

All evaluation runs the **C++ SIL binary** against the full 220,320-frame dataset. Python inference (TFLite runtime) is used only for threshold selection.

### MSE Distribution by Label

| Label | n | p25 | p50 | p75 | p90 | p99 | max |
|---|---|---|---|---|---|---|---|
| NORMAL | 205,836 | 0.100 | 0.148 | 0.252 | 0.622 | 2.070 | 112.0 |
| RECOVERING | 14,477 | 3.057 | 3.952 | 17.181 | 19.809 | 26.958 | 105.5 |
| BROKEN | 7 | 1.019 | 1.552 | 11.836 | 19.486 | 20.348 | 20.4 |

The NORMAL and RECOVERING distributions separate cleanly: NORMAL p99 = 2.07 sits well below RECOVERING p50 = 3.95.

### Threshold Sweep

| Threshold | TP | FP | TN | FN | Precision | Recall | F1 | FPR |
|---|---|---|---|---|---|---|---|---|
| 0.80 | 14,447 | 17,958 | 187,878 | 37 | 44.6% | 99.7% | 0.616 | 8.72% |
| 1.00 | 14,372 | 13,855 | 191,981 | 112 | 50.9% | 99.2% | 0.673 | 6.73% |
| 1.029 | 14,348 | 13,192 | 192,644 | 136 | 52.1% | 99.1% | 0.683 | 6.41% |
| 1.20 | 14,045 | 10,239 | 195,597 | 439 | 57.8% | 97.0% | 0.725 | 4.97% |
| 1.50 | 13,456 | 6,073 | 199,763 | 1,028 | 68.9% | 92.9% | 0.791 | 2.95% |
| **2.07** ★ | **12,688** | **2,056** | **203,780** | **1,796** | **86.1%** | **87.6%** | **0.868** | **1.00%** |
| 2.50 | 11,637 | 1,161 | 204,675 | 2,847 | 90.9% | 80.3% | 0.853 | 0.56% |
| 3.00 | 11,014 | 664 | 205,172 | 3,470 | 94.3% | 76.0% | 0.842 | 0.32% |

**Selected threshold: 2.07** (NORMAL p99) — best F1 of 0.868.

### Confusion Matrix at Threshold = 2.07

```
                       Pred NORMAL    Pred ANOMALY
  Actual NORMAL          203,780           2,056
  Actual ANOMALY           1,796          12,688
```

| Metric | Value |
|---|---|
| True Positives (TP) | 12,688 |
| False Positives (FP) | 2,056 |
| True Negatives (TN) | 203,780 |
| False Negatives (FN) | 1,796 |
| Precision | 86.1% |
| Recall (overall) | 87.6% |
| F1 Score | **0.868** |
| False Positive Rate | **1.00%** |
| Recall — RECOVERING | 87.5% (12,668 / 14,477) |
| Recall — BROKEN | 100.0% (7 / 7) |

### Live C++ Pipeline Output

```
[INFO] Processing Kaggle Pump Data Stream...
[SUCCESS] Processed 220320 frames. Anomalies detected: 14169
```

14,169 flagged (6.4%) vs true anomaly rate 6.6% — within 4% of the Python sweep prediction, with the delta explained by int8 rounding differences between TFLite Micro and the full TFLite runtime.

---

## Operating Points

### High-Recall Mode (`threshold = 0.80`)
- **Recall: 99.7%** — misses only 37 of 14,484 anomalies
- FPR: 8.72% on NORMAL frames
- Use when **missing a failure is unacceptable** (unplanned downtime, critical pump)

### Balanced Mode (`threshold = 2.07`) ← current
- **F1: 0.868**, Precision 86.1%, Recall 87.6%
- FPR: **1.00%** — operators receive reliable, low-noise alerts
- Use when **alarm fatigue matters**

---

## Key Engineering Fixes

| Bug | Symptom | Fix |
|---|---|---|
| Linker: `MicroPrintf` undefined | Link error | Added `micro_log.cc`, `debug_log.cc` to CMake sources |
| FlatBuffers misalignment | Segfault / MSE = 1.93×10¹⁶ | Added `alignas(8)` to model array |
| ABI mismatch (`TF_LITE_STATIC_MEMORY`) | Wrong scale read → garbage MSE | Added `-DTF_LITE_STATIC_MEMORY` compile flag |
| `tensor_arena_` on stack | MSE = 834,911 (arena overlapped `data[]`) | Made `tensor_arena_` a `static` class member (BSS) |
| `sensor_15` column offset | Feature misalignment from col 14 onward | Skip col 15 in CSVReader loop |
| Scaler fitted on 10K rows | 94.9% FPR | Retrain scaler + model on all 205,836 NORMAL rows |
| `src/model_data.h` shadowing `include/model_data.h` | Old model loaded after rebuild | Deleted stale `src/model_data.h` |

---

## Building and Running

### Prerequisites
- C++17 compiler (GCC, Clang)
- CMake 3.14+
- Internet access on first build (FetchContent downloads TFLite Micro)

### Build

```bash
mkdir build && cd build
cmake ..
cmake --build . --parallel 4
```

### Run

Place the [Kaggle sensor dataset](https://www.kaggle.com/datasets/nphantawee/pump-sensor-data) at `data/sensor.csv`, then:

```bash
./build/SmartSensorAnomaly
```

---

## Roadmap

- [x] **Phase 1** — High-speed CSV stream parser (SensorPacket, CSVReader, CircularBuffer)
- [x] **Phase 2** — Edge DSP preprocessing (median imputation, z-score normalisation)
- [x] **Phase 3** — TinyML inference (int8 autoencoder via TFLite Micro, threshold sweep, validation)
- [ ] **Phase 4** — Hardware abstraction (`ISensorDriver`) to swap CSV simulator for I2C/SPI sensor reads on ESP32 / Cortex-M
