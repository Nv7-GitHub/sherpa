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
float mx, my, mz;
float gx, gy, gz;

bool mpuRead() {
  if (!IMU.gyroscopeAvailable()) {return false;}
  if (!IMU.accelerationAvailable()) {return false;}
  if (!IMU.magneticFieldAvailable()) {return false;}

  IMU.readGyroscope(gx, gy, gz);
  IMU.readAcceleration(ax, ay, az);
  IMU.readMagneticField(mx, my, mz);

  // Filter
  filter.update(gx, gy, gz, ax, ay, az, mx, my, mz);

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
  return ax;
}

float accely() {
  return ay;
}

float accelz() {
  return az;
}

#endif
