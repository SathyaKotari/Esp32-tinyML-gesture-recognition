#include <Arduino.h>
#include <Wire.h>

class DataCollect {
    public:
        int16_t accelerometer_x, accelerometer_y, accelerometer_z;
        int16_t gyro_x, gyro_y, gyro_z;
        int16_t temp;
        float modelData[6];

        //Gyroscope Offset Correction
        float gyro_xO = 0, gyro_yO = 0, gyro_zO = 0;
        //Accelerometer Offset Correction
        float accel_xO = 0, accel_yO = 0, accel_zO = 0;
        //Sums for Offset Calculation
        int sumGx = 0;
        int sumGy = 0;
        int sumGz = 0;
        int sumAx = 0;
        int sumAy = 0;
        int sumAz = 0;


        DataCollect(){
            Serial.print("Function was called");
        }

        void collectData(int MPUAddress){
            Wire.beginTransmission(MPUAddress); // I2C address of the MPU-6050
            Wire.write(0x3B); // Starting register for Accel Readings
            Wire.endTransmission(false);
            Wire.requestFrom((uint16_t)MPUAddress, (uint8_t)14, true);

            // Read accelerometer data
            accelerometer_x = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
            accelerometer_y = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
            accelerometer_z = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)

            // Read gyroscope data
            gyro_x = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
            gyro_y = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
            gyro_z = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

        }
        void calibrateData(int MPUAddress){
            for(int i = 0; i< 250; i++){
                collectData(MPUAddress);
                sumGx = sumGx + gyro_x;
                sumGy = sumGy + gyro_y;
                sumGz = sumGz + gyro_z;
                sumAx = sumAx + accelerometer_x;
                sumAy = sumAy +  accelerometer_y;
                sumAz = sumAz + accelerometer_z;
            }
            //setting offset
            gyro_xO = sumGx/250.0;
            gyro_yO = sumGy/250.0;
            gyro_zO = sumGz/250.0;
            accel_xO = sumAx/250.0;
            accel_yO = sumAy/250.0;
            accel_zO = (sumAz/250.0) - 16384; // we expect the raw value when stationary to be 16384 
        }

        float* cleanRaw(){
            //Divide values by sensitivity scalars
            //Accelerometer values divided by 16384
            modelData[0] = (accelerometer_x - accel_xO) / 16384.0f;
            modelData[1] = (accelerometer_y - accel_yO) / 16384.0f;
            modelData[2] = (accelerometer_z - accel_zO) / 16384.0f;

            //Gyroscope values divided by 131
            modelData[3] = (gyro_x- gyro_xO) / 131.0f;
            modelData[4] = (gyro_y- gyro_yO) / 131.0f;
            modelData[5] = (gyro_z- gyro_zO) / 131.0f;
            return modelData;
        }


        // Getter methods
        int16_t getAccelerometerX() const {
            return accelerometer_x;
        }
        int16_t getAccelerometerY() const {
            return accelerometer_y;
        }
        int16_t getAccelerometerZ() const {
            return accelerometer_z;
        }
        int16_t getGyroX() const {
            return gyro_x;
        }
        int16_t getGyroY() const {
            return gyro_y;
        }
        int16_t getGyroZ() const {
            return gyro_z;
        }

};