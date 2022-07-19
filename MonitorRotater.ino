/*
   Author           : Shenggao Li
   Date             : 2022 July
   Target Board     : Arduino Pro Micro
   Peripherals      : ADXL345                       x 1
                        Sensing the rotation
                      Touch Sensor (or any button)  x 1
                        As the major input device
                      Toggle Switch                 x 1
                        Use as a enable/disable switch
   Compatible with  : Arduino Micro, etc.

*/


#include <Wire.h>  // Wire library - used for I2C communication
#include <EEPROM.h>
#include "Keyboard.h"
#include "helper.hpp"

//#define DEBUG
#define KEYBOARD

#define CAL_HEADER 0xCA // Calibration data header

// Use USB serial port for debugging
#ifdef DEBUG
#define Serial_port Serial
#else
//#define Serial_port Serial1
// Seems USB serial can work with Keyboard port at the same time.
// Use Serial1 if you have any stability issue.
#define Serial_port Serial
#endif //DEBUG

// Define pin connection
#define toggle 5
#define button 4

int ADXL345 = 0x53; // The ADXL345 sensor I2C address
// Sample number for calibration
#define NUM_REST 1000
// Sample delay time (in ms)
#define DELAY_REST 5

// Some process control vars
static bool last_button_state = false;
static uint32_t reset_millis = 0;
bool resetted = false;
static uint32_t check_millis = 0;

// Init vector and matrix variables
Vec3D g_vec(0, -1, 0);
Vec3D g_ref(0, -1, 0);

const Mat Rot_90 (0, 1, 0, -1, 0, 0, 0, 0, 1); // ANGLEFT
const Mat Rot_180(-1, 0, 0, 0, -1, 0, 0, 0, 1); // ANGUP
const Mat Rot_270(0, -1, 0, 1, 0, 0, 0, 0, 1); // ANGRIGHT


// enum for 4 rotation configs
enum DirecAngle { ANGDOWN, ANGLEFT, ANGUP, ANGRIGHT };
// Init rotation angle
DirecAngle rotAngle_curt(ANGDOWN), rotAngle_last(ANGDOWN);



void setup() {
  // put your setup code here, to run once:
  Serial_port.begin(9600);

#ifdef KEYBOARD
  Keyboard.begin();
#endif //KEYBOARD
  delay(2000);
  Serial_port.println("Starting Device...");
  Serial_port.println("Waiting for USB connection ...");
  delay(3000);

  Wire.begin(); // Initiate the Wire library
  IMU_offset();

  // Set ADXL345 in measuring mode
  Wire.beginTransmission(ADXL345); // Start communicating with the device
  Wire.write(0x2D); // Access/ talk to POWER_CTL Register - 0x2D
  // Enable measurement
  Wire.write(8); // (8dec -> 0000 1000 binary) Bit D3 High for measuring enable
  Wire.endTransmission();
  delay(10);

  pinMode(toggle, INPUT_PULLUP);
  pinMode(button, INPUT);
  resetted = true;

  load_g_ref(); // Load calibrated data from EEPROM
  
  Serial_port.println("[INITIALIZATION DONE]");
  
}

void loop() {
  const uint32_t t = millis();
  const bool button_state = digitalRead(button);
  if ( button_state ) { // button pressed
    if (!last_button_state) {
      reset_millis = t + 3000; // set 3 seconds for restart;
    } else {
      if (t > reset_millis && resetted) {
        resetted = false;
        reset_g();
      }
    }

  } else {
    if (last_button_state) {
      if (!resetted) {
        resetted = true;
      } else {
        g_vec = read_gravity();
        rotAngle_curt = get_angle();
        Serial_port.print("Rotation angle is ");
        Serial_port.println(rotAngle_curt);
        rotate();
        rotAngle_last = rotAngle_curt;
      }
    }
  }
  last_button_state = button_state;

  if (t > check_millis) {
    check_millis = t + 3000; // check orientation per 3 secs
    g_vec = read_gravity();
    rotAngle_curt = get_angle();
    Serial_port.print("Rotation angle is ");
    Serial_port.println(rotAngle_curt);

    if (rotAngle_curt != rotAngle_last) {
      rotate();
      rotAngle_last = rotAngle_curt;
    }
  }

}
