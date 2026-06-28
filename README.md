# Embedded Pump Anomaly Detector

> Catching pump failures before they happen — on a microcontroller, in real time, with no cloud required.

<img width="1024" height="559" alt="SIL Architecture" src="https://github.com/user-attachments/assets/21de00cb-ad8d-4acf-97d8-210e5b130439" />

---

## The Problem

Industrial pumps fail. When they do, it's expensive — unplanned downtime, damaged equipment, sometimes dangerous situations. The telltale signs are almost always there in the sensor data beforehand: subtle drifts in pressure, temperature, flow rate. The question is whether anything is *listening*.

The obvious answer is to throw an ML model at it. The less obvious problem: the sensors live on edge hardware — ESP32s, ARM Cortex-M chips — with kilobytes of RAM and no internet connection. You can't run a Python server next to a pump in a factory floor.

This project is the answer to that problem.

---

## The Approach: Test on the Laptop, Deploy to the Chip

Flashing experimental ML onto live industrial hardware during development is a terrible idea. Instead, this project uses a **Software-in-the-Loop (SIL)** strategy:

The C++ inference engine treats each row of a CSV file as if it were a live sensor interrupt. The pump data (220,320 real readings from the [Kaggle Water Pump dataset](https://www.kaggle.com/datasets/nphantawee/pump-sensor-data)) streams through the pipeline exactly the way real I2C sensor bursts would on physical hardware.

The payoff: **95% of the codebase is identical between the laptop simulation and the embedded target.** When it's time to deploy to an ESP32, only the low-level data source changes — everything from preprocessing to inference stays untouched.

---

## How It Works

### The Model: Teaching the Pump What "Normal" Looks Like

The detector is an **autoencoder** — a neural network trained to compress and reconstruct normal pump sensor readings. When the pump is healthy, it reconstructs well. When something goes wrong, reconstruction error (MSE) spikes. That spike is the alarm.

```
51 sensors → Dense(16) → Dense(8) → Dense(16) → 51 outputs
```

The model is quantized to **int8** so it fits on a microcontroller. The full precision weights compress down to an arena that fits in 60 KB of static RAM — no heap allocation, no dynamic memory, no fragmentation.

### The Preprocessing Pipeline

Before any sensor reading reaches the model, it goes through three steps — the same three steps used during training:

1. **Median imputation** — dead or missing sensors get replaced with their historical median (computed from 205,836 normal frames)
2. **Z-score normalisation** — each sensor scaled to zero mean, unit variance
3. **Int8 quantisation** — floats clamped and mapped to `[-128, 127]` for the TFLite Micro runtime

---

## Results

Evaluated against the full 220,320-frame dataset using the C++ SIL binary:

| Metric | Value |
|---|---|
| F1 Score | **0.868** |
| Precision | 86.1% |
| Recall | 87.6% |
| False Positive Rate | **1.0%** |
| Recall — BROKEN frames | **100%** (7/7) |

At the operating threshold (MSE = 2.07), the pipeline flagged 14,169 anomalies — within 4% of the Python reference sweep, with the gap explained entirely by int8 rounding differences.

### The False Positive Story

The first model had a **94.9% false positive rate** — nearly every normal frame triggered an alarm. The culprit: the scaler was fitted on only the first ~10,000 rows of training data, missing several pump operating regimes entirely. One sensor (`sensor_19`) ranged from 249 to 664 across the dataset, but the narrow scaler had fitted a std of 5.03. Later frames produced z-scores of **−82** and the model had never seen anything like them.

The fix: refit the scaler and retrain on all 205,836 normal frames. `sensor_19`'s std went from 5.03 to 205.05. The FPR dropped from 94.9% to 1.0%.

---

## Two Operating Modes

Depending on the deployment context, you can tune the threshold:

**Balanced** (`threshold = 2.07`) — current default
- F1: 0.868, FPR: 1.0%
- Operators get reliable, low-noise alerts

**High-Recall** (`threshold = 0.80`)
- Recall: 99.7%, misses only 37 of 14,484 anomalies
- For situations where missing a failure is unacceptable

---

## Build & Run

**Prerequisites:** GCC or Clang (C++17), CMake 3.14+, internet on first build (FetchContent pulls TFLite Micro).

```bash
mkdir build && cd build
cmake ..
cmake --build . --parallel 4
```

Place the [Kaggle sensor CSV](https://www.kaggle.com/datasets/nphantawee/pump-sensor-data) at `data/sensor.csv`, then:

```bash
./build/SmartSensorAnomaly
```

Expected output:
```
[INFO] Processing Kaggle Pump Data Stream...
[SUCCESS] Processed 220320 frames. Anomalies detected: 14169
```

---

## Roadmap

- [x] Phase 1 — CSV stream parser (`SensorPacket`, `CSVReader`, `CircularBuffer`)
- [x] Phase 2 — Edge DSP preprocessing (median imputation, z-score, int8 quantisation)
- [x] Phase 3 — TinyML inference (int8 autoencoder, threshold sweep, full dataset validation)
- [ ] Phase 4 — Hardware abstraction (`ISensorDriver`) to swap the CSV simulator for real I2C/SPI reads on ESP32 / Cortex-M
