#if defined(ARDUINO_ARCH_NRF52840)

// Useful source: https://github.com/njanssen/arduino-nano-33-ble/blob/master/Nano33BLEService/Nano33BLEService.ino

#include <Arduino_LSM9DS1.h>
#include <MadgwickAHRS.h>

Madgwick filter;

void setupMpu() {
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU");
  }
  filter.begin(119); // IMU hz
}


float ax, ay, az;
//float mx, my, mz;
float gx, gy, gz;

bool mpuRead() {
  /*if (!IMU.gyroscopeAvailable()) {return false;}
  if (!IMU.accelerationAvailable()) {return false;}
  if (!IMU.magneticFieldAvailable()) {return false;}*/
  if (!IMU.readGyroscope(gx, gy, gz)) {return false;}
  if (!IMU.readAcceleration(ax, ay, az))  {return false;}
  //if (!IMU.readMagneticField(mx, my, mz)) {return false;}
  

  // Filter
  //filter.update(gx, gy, gz, ax, ay, az, my, mz, mx);
  filter.updateIMU(gx, gy, gz, ax, ay, az);
  ax += 0.02;
  ay += 0.01;
  az += 0.01;
  gx -= 0.3;
  gy += 0;
  gz += 0.6;
  

  return true;
}

float yaw() {
  return filter.getYaw();
}

float pitch() {
  return filter.getPitch();

}

float roll() {
  return filter.getRoll();
}

float accelx() {
  return ax*9.807;
}

float accely() {
  return ay*9.807;
}

float accelz() {
  return az*9.807;
}

#endif
