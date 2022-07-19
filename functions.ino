

void IMU_offset() {
  // Off-set Calibration
  // The data was manually obtained for my IMU
  // Should be changed to match the IMU offset for better accuracy
  // NOTE: it should be fine even without the offset

  //X-axis
  Wire.beginTransmission(ADXL345);
  Wire.write(0x1E);  // X-axis offset register
  Wire.write(-4);
  Wire.endTransmission();
  delay(10);

  //Y-axis
  Wire.beginTransmission(ADXL345);
  Wire.write(0x1F); // Y-axis offset register
  Wire.write(0);
  Wire.endTransmission();
  delay(10);

  //Z-axis
  Wire.beginTransmission(ADXL345);
  Wire.write(0x20); // Z-axis offset register
  Wire.write(-7);
  Wire.endTransmission();
  delay(10);
}


DirecAngle get_angle() {
  // Find the best matching rotation configuration

  // Using dot product as the index
  double direc_index[4];
  direc_index[DirecAngle::ANGDOWN] = g_ref * g_vec;
  direc_index[DirecAngle::ANGLEFT] = (Rot_90 * g_ref) * g_vec;
  direc_index[DirecAngle::ANGUP] = (Rot_180 * g_ref) * g_vec;
  direc_index[DirecAngle::ANGRIGHT] = (Rot_270 * g_ref) * g_vec;

  // Find the highest direction
  DirecAngle rotAngle = ANGDOWN;
  double index_max = direc_index[0];
  for (int i = 1; i < 4; ++i) {
    if (direc_index[i] > index_max) {
      index_max = direc_index[i];
      rotAngle = i;
    }
  }
  return rotAngle;
}


void rotate() {
  // Send rotate command to PC/MAC
  
  if (digitalRead(toggle)) { // If auto-rotate mode is enabled

    Serial_port.println("Rotating...");
#ifdef KEYBOARD
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press(KEY_LEFT_SHIFT);
    Keyboard.press(KEY_LEFT_ALT);
    Keyboard.press(KEY_LEFT_GUI);

    switch (rotAngle_curt) {
      case ANGDOWN:
        Keyboard.press(KEY_DOWN_ARROW);
        break;
      case ANGLEFT:
        Keyboard.press(KEY_LEFT_ARROW);
        break;
      case ANGRIGHT:
        Keyboard.press(KEY_RIGHT_ARROW);
        break;
      case ANGUP:
        Keyboard.press(KEY_UP_ARROW);
        break;
      default:
        Keyboard.press(KEY_DOWN_ARROW);

    }

    Keyboard.releaseAll();
#endif //KEYBOARD
  }
}


Vec3D read_gravity() {
  // Get the current gravity vector (assuming static)

  // === Read acceleromter data === //
  float X_out, Y_out, Z_out;  // Outputs
  Wire.beginTransmission(ADXL345);
  Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  X_out = ( Wire.read() | Wire.read() << 8); // X-axis value
  X_out = X_out / 256; //For a range of +-2g, we need to divide the raw values by 256, according to the datasheet
  Y_out = ( Wire.read() | Wire.read() << 8); // Y-axis value
  Y_out = Y_out / 256;
  Z_out = ( Wire.read() | Wire.read() << 8); // Z-axis value
  Z_out = Z_out / 256;
  return -Vec3D(X_out, Y_out, Z_out);
}

void load_g_ref() {
  // Load calibrated data from EEPROM

  if (EEPROM.read(0) == CAL_HEADER) { // If the header matches
    Serial_port.println("Loading calibration data...");

    for (int i = 0; i < vec_size; ++i) {
      vec_union.vec_char[i] = EEPROM.read(i + 1);
    }
    g_ref = vec_union.vec_var;

    disp_g_ref();
  } else {
    Serial_port.println("No calibration data found. Using default value.");
  }

}

void save_g_ref() {
  // Save calibrated data to EEPROM
  Serial_port.println("Saving calibration data...");
  vec_union.vec_var = g_ref;
  EEPROM.write(0, CAL_HEADER); // Write the header first
  for (int i = 0; i < vec_size; ++i) {
    EEPROM.write(i + 1, vec_union.vec_char[i]);
  }
}

void disp_g_ref() {
  // Serial print reference "down pointing" gravity vector

  Serial_port.print("Xref= ");
  Serial_port.print(g_ref.x);
  Serial_port.print("   Yref= ");
  Serial_port.print(g_ref.y);
  Serial_port.print("   Zref= ");
  Serial_port.println(g_ref.z);
}

void reset_start_command() {
  // Call the computer to show calibration message

#ifdef KEYBOARD
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press(KEY_LEFT_SHIFT);
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('0');
  delay(100);
  Keyboard.releaseAll();
#endif //KEYBOARD

}
void reset_end_command() {
  // Call the computer to end the calibration message

#ifdef KEYBOARD
  Keyboard.press(KEY_ESC);
  Keyboard.releaseAll();
#endif //KEYBOARD

}

void reset_g() {
  // Calibrate the reference "down pointing" gravity vector
  reset_start_command();
  
  Serial_port.println("Resetting ...");
  // Sample 
  Vec3D g_REST(0, 0, 0);
  for (int i = 0; i < NUM_REST; ++i) {
    g_REST = g_REST + read_gravity();
    delay(DELAY_REST);
  }
  g_ref = (1.0 / NUM_REST) * g_REST;

  disp_g_ref();

  save_g_ref();

  reset_end_command();
}
