import pandas as pd
import numpy as np
import tensorflow as tf
import os

SeqLen = 50
StepSize = 10
COLUMNS = ["AccelX", "AccelY", "AccelZ", "GyroX", "GyroY", "GyroZ"]
script_dir = os.path.dirname(os.path.abspath(__file__))
data_dir = os.path.join(script_dir, 'data')

#LOAD DATA
idleData = pd.read_csv(os.path.join(data_dir, 'IdleData.csv'), names=COLUMNS, header=None)
punchData = pd.read_csv(os.path.join(data_dir, 'PunchData.csv'), names=COLUMNS, header=None)
waveData = pd.read_csv(os.path.join(data_dir, 'WaveData.csv'), names=COLUMNS, header=None)

idleData[['AccelX', 'AccelY', 'AccelZ']] /= 4.0
idleData[['GyroX', 'GyroY', 'GyroZ']] /= 2000.0

punchData[['AccelX', 'AccelY', 'AccelZ']] /= 4.0
punchData[['GyroX', 'GyroY', 'GyroZ']] /= 2000.0

waveData[['AccelX', 'AccelY', 'AccelZ']] /= 4.0
waveData[['GyroX', 'GyroY', 'GyroZ']] /= 2000.0

#Breaks data up into the clips
def createSegmentsLabels(df, label, time_steps, step):
    segments = []
    labels = []
    # Loop through the data
    for i in range(0, len(df) - time_steps, step):
        xs = df['AccelX'].values[i: i + time_steps]
        ys = df['AccelY'].values[i: i + time_steps]
        zs = df['AccelZ'].values[i: i + time_steps]
        gxs = df['GyroX'].values[i: i + time_steps]
        gys = df['GyroY'].values[i: i + time_steps]
        gzs = df['GyroZ'].values[i: i + time_steps]
        
        # Create a single example
        segments.append([xs, ys, zs, gxs, gys, gzs])
        labels.append(label)

    # Reshape segments to (Number_of_Windows, Time_Steps, Features)
    reshaped_segments = np.array(segments, dtype=np.float32).transpose(0, 2, 1)
    return reshaped_segments, np.array(labels)

# using above method label each data set
x_idle, y_idle = createSegmentsLabels(idleData, 0, SeqLen, StepSize)
x_punch, y_punch = createSegmentsLabels(punchData, 1, SeqLen, StepSize)
x_wave, y_wave = createSegmentsLabels(waveData, 2, SeqLen, StepSize)


#Stack Data
X = np.concatenate((x_idle, x_punch, x_wave))
y = np.concatenate((y_idle, y_punch, y_wave))

# --- THE FIX: Manually Flatten the Data ---
# We turn the 3D matrix (N, 50, 6) into a 2D matrix (N, 300) here.
# This allows us to delete the 'Flatten' layer from the model later.
total_inputs = SeqLen * 6
X = X.reshape(X.shape[0], total_inputs)

# Use keras here
y = tf.keras.utils.to_categorical(y)

print("--- DATA PROCESSING COMPLETE ---")
print(f"Total Inputs (X) Shape: {X.shape}")
print(f"Total Labels (y) Shape: {y.shape}")
print("\nExample: If X shape is (500, 50, 6), you have 500 clips, each 50 samples long, with 6 sensors.")


####
#Convert to tflite
from sklearn.model_selection import train_test_split
from tensorflow import keras
from tensorflow.keras import layers

print("\n--- SPLITTING DATA ---")
# Split into training (80%) and testing (20%)
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

print("--- BUILDING MODEL ---")
model = keras.Sequential([
    # 1. Dense Layer (The Input is merged here)
    # We removed layers.Flatten(). 
    # We removed layers.Input(shape=(SeqLen, 6)).
    # We now tell it to expect a flat list of 300 numbers directly.
    layers.Dense(32, activation='relu', input_shape=(total_inputs,)),

    # 2. Dropout (Randomly forget data to prevent memorization)
    layers.Dropout(0.2),

    # 3. Output Layer (3 classes)
    layers.Dense(3, activation='softmax') 
])

model.compile(optimizer='adam', loss='categorical_crossentropy', metrics=['accuracy'])

print("\n--- TRAINING STARTING ---")
# This is where the learning happens. 
# watch 'accuracy' go UP and 'loss' go DOWN.
history = model.fit(X_train, y_train, epochs=30, batch_size=16, validation_data=(X_test, y_test))


print("\n--- CONVERTING TO TINYML ---")
# Convert to TensorFlow Lite
def representative_dataset():
    for i in range(100):
        yield [X_train[i:i+1].astype(np.float32)]

converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.representative_dataset = representative_dataset
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8

tflite_model = converter.convert()

# Save the file 
tflite_filename = os.path.join(script_dir, 'gestureModel.tflite')
with open(tflite_filename, 'wb') as f:
    f.write(tflite_model)

print(f"\nSUCCESS! Model saved to: {tflite_filename}")
print(f"Model Size: {len(tflite_model)} bytes")


##################################################
#Convert .tflite to a c file
import os

# 1. READ THE BINARY FILE
tflite_path = "gestureModel.tflite" 
# (Make sure this matches the file name in your folder)

with open(tflite_path, 'rb') as f:
    hex_data = f.read().hex()

# 2. FORMAT AS C++ CODE
hex_array = [f"0x{hex_data[i:i+2]}" for i in range(0, len(hex_data), 2)]

c_code = f"""
#ifndef GESTURE_MODEL_H
#define GESTURE_MODEL_H

// Auto-generated from {tflite_path}
const unsigned char gesture_model[] = {{
    {', '.join(hex_array)}
}};

const unsigned int gesture_model_len = {len(hex_array)};

#endif
"""

# 3. SAVE AS HEADER
with open("model.h", "w") as f:
    f.write(c_code)

print("DONE! Copy 'model.h' to your PlatformIO src folder.")