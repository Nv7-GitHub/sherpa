// Connect mpu to SDA, SCL, GND, VCC

#include <I2Cdev.h>
#include <MPU6050_6Axis_MotionApps20.h>
#include <Wire.h>

MPU6050 mpu;


// MPU data
uint8_t fifoBuffer[64]; // FIFO storage buffer
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector

// Values
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

// Code
void setupMpu() {
  Wire.begin();
  mpu.initialize();
  //Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
  mpu.dmpInitialize();

  // MPU Calibrate
  mpu.CalibrateAccel(6);
  mpu.CalibrateGyro(6);
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
  // Gets world-relative accel
  mpu.dmpGetQuaternion(&q, fifoBuffer);
  mpu.dmpGetAccel(&aa, fifoBuffer);
  mpu.dmpGetGravity(&gravity, &q);
  mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
  mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
}

// Read methods
float yaw() {
  return ypr[0];
}

float pitch() {
  return ypr[1];
}

float roll() {
  return ypr[2];
}

float accelx() {
  return aaWorld.x;
}

float accely() {
  return aaWorld.y;
}

float accelz() {
  return aaWorld.z;
}
