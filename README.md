# Embedded Pump Predictive Maintenance Engine

An industrial-grade, zero-allocation C++ simulation and stream-processing engine designed for real-time anomaly detection on smart edge sensors. 

This project bridges the gap between high-dimensional industrial sensor streams (simulated using the Kaggle Water Pump dataset) and the highly constrained environments of edge microcontrollers (such as the ESP32 or ARM Cortex-M architectures).

---

## 🚀 The Philosophy: Hardware-in-the-Loop Simulation
Deploying machine learning prototypes directly onto high-voltage, physical industrial motors is impractical and risky during early-stage development. 

This project implements a **Software-in-the-Loop (SIL)** paradigm. The core C++ processing engine treats lines of a massive time-series dataset as a real-time hardware data bus interrupt. Because the data processing layer is decoupled from the ingestion layer, **95% of this codebase remains identical when flashed onto a physical microcontroller**; only the low-level peripheral driver changes.
