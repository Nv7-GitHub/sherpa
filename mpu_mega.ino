#if defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_MEGA)

// Connect mpu to SDA, SCL, GND, VCC

#include <I2Cdev.h>
#include <MPU6050_6Axis_MotionApps20.h>
#include <Wire.h>

MPU6050 mpu;


// MPU data
uint8_t fifoBuffer[64]; // FIFO storage buffer
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector

// Values
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

// Code
void setupMpu() {
  Wire.begin();
  mpu.initialize();
  //Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
  mpu.dmpInitialize();

  // MPU Calibrate
  mpu.CalibrateAccel(60);
  mpu.CalibrateGyro(60);
  mpu.PrintActiveOffsets();
  mpu.setDMPEnabled(true);
}


bool mpuRead() {
  if (!mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) {
    return false;
  }
  
  readYpr();
  readAccel();

  return true;
}

void readYpr() {
  mpu.dmpGetQuaternion(&q, fifoBuffer);
  mpu.dmpGetGravity(&gravity, &q);
  mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
}

void readAccel() {
  mpu.dmpGetQuaternion(&q, fifoBuffer);
  mpu.dmpGetAccel(&aa, fifoBuffer);
  mpu.dmpGetGravity(&gravity, &q);
  mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
}

// Read methods
float yaw() {
  return (int)(ypr[0] * RAD_TO_DEG) % 360;
}

/*int pitch() {
  return (int)(ypr[1] * RAD_TO_DEG) % 360;
}

int roll() {
  return (int)(ypr[2] * RAD_TO_DEG) % 360;
}*/

const float accelFactor = (float)(8192) / 9.80665;
/* 
use 9.8 value to convert from gs to m/s^2, find first value through trial and error and below values: 
  
±2g, = (4) we need to use 2 bits 11b for the whole number and the remaining 14 for precision so divide by 16384 or 11 1111 1111 1111b
±4g, = (8) we need to use 3 bits 111b for the whole number and the remaining 13 for precision so divide by 8192 or 1 1111 1111 1111b
±8g, = (16) we need to use 4 bits 1111b for the whole number and the remaining 12 for precision so divide by 4096 or 1111 1111 1111b
±16g. = (32) we need to use 5 bits 11111b for the whole number and the remaining 11 for precision so divide by 2048 or 111 1111 1111b
 */

// These return m/s^2
float accelx() {
  return aaReal.x/accelFactor;
}

float accely() {
  return aaReal.y/accelFactor;
}

/*float accelz() {
  return aaReal.z/accelFactor;
}*/
#endif
