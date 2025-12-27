#include <Arduino.h>
#include "DataCollect.cpp"
#include "model.h"
#include <EloquentTinyML.h>

// --- CONFIGURATION ---
#define SAMPLE_RATE 50       // Hz
#define DELAY_MS 20          // 1000ms / 50Hz = 20ms
#define SAMPLES_PER_GESTURE 50
#define AXES 6               // Ax, Ay, Az, Gx, Gy, Gz

// 50 samples * 6 axes = 300 inputs total
#define NUMBER_OF_INPUTS (SAMPLES_PER_GESTURE * AXES)
#define NUMBER_OF_OUTPUTS 3  // Idle, Punch, Wave
#define TENSOR_ARENA_SIZE (64 * 1024)

DataCollect sensor;
Eloquent::TinyML::TfLite<NUMBER_OF_INPUTS, NUMBER_OF_OUTPUTS, TENSOR_ARENA_SIZE> ml;

const int MPU_ADDR = 0x68;
DataCollect rawData;
float* cleanDataArray;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Wire.begin(); // Starts the i2c protocol
  Wire.setClock(400000);//Max speed

  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board) at the set speed
  Wire.write(0x6B); // PWR_MGMT_1 register write to this adress
  Wire.write(0x00); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  rawData.calibrateData(MPU_ADDR);

  Serial.print("Free heap before ML: ");
  Serial.println(ESP.getFreeHeap());

  if (!ml.begin(gesture_model)) {
      Serial.println("ML init failed (AllocateTensors)");
      Serial.print("Free heap at failure: ");
      Serial.println(ESP.getFreeHeap());
      while (true);  // STOP HERE
  }

  Serial.print("Free heap after ML: ");
  Serial.println(ESP.getFreeHeap());
}

void loop() {
  // put your main code here, to run repeatedly:
  rawData.collectData(MPU_ADDR);
  cleanDataArray = rawData.cleanRaw();

  float input_buffer[NUMBER_OF_INPUTS];
  Serial.println("Recording Gesture");

  for (int i = 0; i < SAMPLES_PER_GESTURE; i++) {
        unsigned long start_time = millis();

        // Get the single moment of data
        rawData.collectData(MPU_ADDR);
        cleanDataArray = rawData.cleanRaw(); // Returns [ax, ay, az, gx, gy, gz]

        int index = i * AXES; 

        // 2. NORMALIZE IT (Match your Python Math!)
        // Assuming cleanDataArray returns Gs and Deg/s
        // If cleanDataArray returns RAW ints, use larger dividers (e.g. 16384.0)
        input_buffer[index + 0] = cleanDataArray[0] / 4.0f;    // Accel X
        input_buffer[index + 1] = cleanDataArray[1] / 4.0f;    // Accel Y
        input_buffer[index + 2] = cleanDataArray[2] / 4.0f;    // Accel Z
        input_buffer[index + 3] = cleanDataArray[3] / 2000.0f; // Gyro X
        input_buffer[index + 4] = cleanDataArray[4] / 2000.0f; // Gyro Y
        input_buffer[index + 5] = cleanDataArray[5] / 2000.0f; // Gyro Z

        // Ensure we only take a sample every 20ms (50Hz)
        while (millis() - start_time < DELAY_MS) {
            // Do nothing
        }
    }

    // send it to the model
    float prediction[NUMBER_OF_OUTPUTS];
    ml.predict(input_buffer, prediction);

    // Find which class had the highest probability
    int best_class = 0;
    float highest_prob = 0.0;

    for (int i = 0; i < NUMBER_OF_OUTPUTS; i++) {
        if (prediction[i] > highest_prob) {
            highest_prob = prediction[i];
            best_class = i;
        }
    }

    Serial.print("Class: ");
    if (best_class == 0) Serial.print("IDLE");
    if (best_class == 1) Serial.print("PUNCH");
    if (best_class == 2) Serial.print("WAVE");

    Serial.print(" (Prob: ");
    Serial.print(highest_prob);
    Serial.println(")");
}
