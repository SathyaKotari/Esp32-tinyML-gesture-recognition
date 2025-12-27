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
