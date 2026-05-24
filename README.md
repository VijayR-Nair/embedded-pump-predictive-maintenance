# Embedded Pump Predictive Maintenance Engine

An industrial-grade, zero-allocation C++ simulation and stream-processing engine designed for real-time anomaly detection on smart edge sensors. 

This project bridges the gap between high-dimensional industrial sensor streams (simulated using the Kaggle Water Pump dataset) and the highly constrained environments of edge microcontrollers (such as the ESP32 or ARM Cortex-M architectures).

---

## 🚀 The Philosophy: Hardware-in-the-Loop Simulation
Deploying machine learning prototypes directly onto high-voltage, physical industrial motors is impractical and risky during early-stage development. 

This project implements a **Software-in-the-Loop (SIL)** paradigm. The core C++ processing engine treats lines of a massive time-series dataset as a real-time hardware data bus interrupt. Because the data processing layer is decoupled from the ingestion layer, **95% of this codebase remains identical when flashed onto a physical microcontroller**; only the low-level peripheral driver changes.
<img width="1024" height="559" alt="image" src="https://github.com/user-attachments/assets/21de00cb-ad8d-4acf-97d8-210e5b130439" />
## 🛠️ Production-Grade C++ Implementation Details

To meet the rigid constraints of real-time embedded environments, this codebase deliberately avoids patterns that cause standard desktop software to fail on edge devices:

* **Zero Dynamic Heap Allocation:** Avoids standard heap usage (`new`, `delete`, `std::vector::push_back`) inside the execution runtime loop. This completely eliminates memory fragmentation, preventing runtime crashes (such as ESP32 Guru Meditation errors) during long-term continuous operation.
* **Cache-Friendly Stack Structures:** Utilizes fixed-size contiguous memory blocks (`std::array`) for high-speed cache lines.
* **FPU Optimization:** Enforces strict 32-bit floating-point precision (`float`), allowing edge devices with hardware Floating Point Units (like the ESP32 or Cortex-M4) to compute calculations in a single clock cycle, skipping expensive software double emulation.
* **Strict Compilation Safety:** Managed entirely via Modern CMake with aggressive warning metrics enabled (`-Wall -Wextra -Wpedantic -Werror`).

---

## 📈 Future Roadmap (The Edge AI Transition)

This repository is designed to evolve sequentially from a desktop simulation to an autonomous edge node:

- [x] **Phase 1: Ingestion Engine** – High-speed, low-overhead stream parser transforming raw multi-column industrial text logs into memory-mapped data structures.
- [ ] **Phase 2: Edge DSP Feature Extraction** – Real-time feature engineering using fixed-size rolling circular buffers to compute sliding averages and variance windows.
- [ ] **Phase 3: TinyML Inference Deployment** – Integrating a compressed, quantized anomaly detection model (Autoencoder/Deep One-Class) via TensorFlow Lite for Microcontrollers.
- [ ] **Phase 4: Hardware Abstraction & Deployment** – Implementing an abstract driver interface (`ISensorDriver`) to swap the desktop CSV simulator for real-world I2C/SPI sensor registers on physical hardware.

---

## 🛠️ Building and Running Locally

### Prerequisites
* A compiler supporting **C++17** or higher (GCC, Clang, or MSVC)
* **CMake** (Version 3.14+)

### Compilation
From the root directory of the project, execute the following commands in your terminal:

```bash
# 1. Create and enter a build compilation directory
mkdir build && cd build

# 2. Configure the project via CMake
cmake ..

# 3. Compile the executable application
cmake --build .

Execution

Ensure you have downloaded your target dataset and placed it under a local directory matching data/sensor.csv. Then execute the compiled binary:
# From the root directory:
./build/SmartSensorAnomaly
