// Connect gps to RX1, TX1, GND, VCC

#include <TinyGPS++.h>

TinyGPSPlus gps;

void setupGps() {
  Serial1.begin(9600);
}

bool gpsRead() {
  while (Serial1.available()) {
    gps.encode(Serial1.read());
  }

  if (satCount() < 3 || alt() == 0) {
    return false;
  }
}

// Read methods
float lat() {
  return gps.location.lat();
}

float lng() {
  return gps.location.lng();
}

double course() {
  return gps.course.deg();  
}

// Altitude in feet
double alt() {
  return gps.altitude.feet();
}

int satCount() {
  return gps.satellites.value();
}
