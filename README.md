# Esp32-tinyML-gesture-recognition

This project implements a gesture recognition system using **TensorFlow Lite for Microcontrollers** on an ESP32. It captures accelerometer and gyroscope data from a **GY-521 (MPU6050)** sensor to classify movements into three categories: **Idle, Punch, and Wave**.
The goal fo the project was to be an intro to TinyML and running a project while cognizant of the hardware limitations

## Hardware
ESP32 Development Board
GY-521 (MPU6050 Accel/Gyro)

## Project Structure
* `training/`: Python scripts to train the Neural Network.
    * `train_model.py`: Loads CSV data, trains the model, and exports `model.h`.
    * `data/`: Contains the `.csv` datasets for Idle, Punch, and Wave.
* `firmware/`: C++ code for the ESP32.
    * `main.cpp`: Main Arduino sketch for inference.
    * `model.h`: The trained model converted to a C byte array.

## How to Run
##1. Collect Data (Optional)
If you want to train your own gestures, use the sensor to log raw X/Y/Z data for Accelerometer and Gyroscope into the `training/data/` folder.

##2. Train the Model
Run the Python script to generate a new model

##3. Run on the board
Run the C++ program on your board
