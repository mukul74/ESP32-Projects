<p align="center">
  <img src="media/demo.gif" width="900">
</p>

<h1 align="center">
ESP32 + MPU6050 Sensor Fusion Visualization
</h1>

<p align="center">
Real-time visualization of Accelerometer, Gyroscope, and Complementary Filter angle estimation using an ESP32 and MPU6050.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/ESP32-WiFi-blue">
  <img src="https://img.shields.io/badge/MPU6050-IMU-green">
  <img src="https://img.shields.io/badge/Sensor-Fusion-orange">
</p>

---

## Overview

This project demonstrates one of the fundamental concepts in robotics, drones, autonomous systems, and inertial navigation:

**How can we estimate orientation using noisy sensor measurements?**

The MPU6050 contains:

* 3-axis Accelerometer
* 3-axis Gyroscope

Both sensors provide useful information, but neither is perfect.

This project streams real-time MPU6050 data from an ESP32 over Wi-Fi to a browser dashboard, allowing direct comparison between:

* Accelerometer-only angles
* Gyroscope-only angles
* Complementary Filter fused angles

The result is an intuitive visualization of sensor fusion in action.

---

## Demo

The animation above shows:

* Accelerometer Roll/Pitch
* Gyroscope Roll/Pitch
* Complementary Filter Roll/Pitch

Observe how:

* Accelerometer signals are noisy
* Gyroscope signals are smooth but drift over time
* Complementary Filter combines the strengths of both

For the full-resolution video:

📹 [demo.mp4](MPU_6050_WIFI_PLANE\ImpOfComplementryFilter.mp4)

---

## Why Sensor Fusion?

### Accelerometer

Advantages:

* No long-term drift
* Uses gravity as a reference

Disadvantages:

* Noisy
* Sensitive to vibration
* Disturbed by motion

---

### Gyroscope

Advantages:

* Smooth
* Fast response
* Excellent short-term motion tracking

Disadvantages:

* Drift accumulates over time

---

### Complementary Filter

The complementary filter combines both measurements:

```cpp
fusedRoll =
    0.98 * (fusedRoll + gx * dt) +
    0.02 * accelRoll;

fusedPitch =
    0.98 * (fusedPitch + gy * dt) +
    0.02 * accelPitch;
```

This allows:

* Gyroscope to capture rapid motion
* Accelerometer to correct long-term drift

Result:

* Smooth orientation estimates
* Stable roll and pitch measurements
* Reduced drift

---

## System Architecture

```text
MPU6050
   │
   ▼
ESP32
   │
   │ Wi-Fi
   ▼
Web Server
   │
   ▼
Browser Dashboard
   │
   ├── Accelerometer Angles
   ├── Gyroscope Angles
   └── Complementary Filter Angles
```

---

## Hardware

### Components

* ESP32 Development Board
* MPU6050 IMU
* Breadboard
* Jumper Wires
* USB Cable

### Wiring

| MPU6050 | ESP32   |
| ------- | ------- |
| VCC     | 3.3V    |
| GND     | GND     |
| SDA     | GPIO 21 |
| SCL     | GPIO 22 |

---

## What You Will Learn

This project is designed for engineers, students, and hobbyists who want to understand:

* IMU fundamentals
* Accelerometer-based angle estimation
* Gyroscope integration
* Sensor noise
* Gyroscope drift
* Sensor Fusion
* Complementary Filters
* Real-time embedded systems
* ESP32 networking
* Browser-based data visualization

---

## Observations

### Accelerometer Only

* Jittery signal
* Noise visible even when stationary
* No drift

### Gyroscope Only

* Very smooth motion
* Accurate short-term tracking
* Drift accumulates over time

### Complementary Filter

* Smooth response
* Drift correction
* Best overall orientation estimate

---

## Current Capabilities

### Supported

* Roll Estimation
* Pitch Estimation
* Real-Time Streaming
* Browser Visualization
* Sensor Fusion

### Not Yet Implemented

* Stable Yaw Estimation
* Magnetometer Integration
* Kalman Filter
* Quaternion Representation
* Data Logging

---

## Results

The project clearly demonstrates the classic IMU tradeoff:

| Method               | Noise | Drift    |
| -------------------- | ----- | -------- |
| Accelerometer        | High  | None     |
| Gyroscope            | Low   | High     |
| Complementary Filter | Low   | Very Low |

The complementary filter provides a practical balance between responsiveness and long-term stability.

---

## Repository

```bash
git clone <your-repository-url>
```

Open the Arduino sketch, configure Wi-Fi credentials, upload to the ESP32, and connect to the hosted dashboard.

---

## Key Takeaway

The accelerometer knows where gravity is.

The gyroscope knows how fast the sensor is rotating.

The complementary filter combines both to produce a significantly more reliable estimate of orientation than either sensor can provide on its own.

This project serves as a foundation for understanding more advanced sensor fusion techniques such as Kalman Filters, Mahony Filters, Madgwick Filters, and full inertial navigation systems.
