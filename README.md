## Demo
Add GIF/video of Idle, Punch, Wave classification.

## System Architecture
MPU6050 → ESP32 firmware → preprocessing → TensorFlow Lite Micro model → gesture output

## Hardware
- ESP32 development board
- GY-521 / MPU6050 accelerometer + gyroscope
- USB power
- Jumper wires / breadboard

## Model
- Classes: Idle, Punch, Wave
- Input features: accelerometer + gyroscope values
- Training data: [number] samples per class
- Accuracy: [add after measuring]
- Inference latency: [add after measuring]
- Model size: [add after measuring]

## How to Run
1. Flash firmware using PlatformIO / Arduino IDE
2. Collect sensor data
3. Train model
4. Convert model to `model.h`
5. Upload inference firmware to ESP32

## What I Learned
- Sensor data quality matters more than expected
- Embedded ML requires memory and latency tradeoffs
- Hardware constraints change the ML workflow
