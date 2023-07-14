double latP;
double lngP;

// Course prediction
double courseP;
double yawOriginal;

// Position prediction
double velX = 0;
double velY = 0;
unsigned long lastPrediction = 0;

// GPS position comparing
double lastLat;
double lastLng;
unsigned long lastGpsUpdate = 0;

void gpsHasBeenUpdated() {
  unsigned long currentTime = millis();
  if (lastGpsUpdate != 0) {
    // Fix velocities
    double deltaT = ((double)(currentTime - lastGpsUpdate)/1000);
    velX = (lng() - lastLng) * deltaT * 111111.0;
    velY = (lat() - lastLat) * deltaT * 111111.0;
    lastGpsUpdate = currentTime;
  } else {
    lastGpsUpdate = currentTime;
  }
  
  
  yawOriginal = yaw();
  latP = lat();
  lngP = lng();
  lastLat = lat();
  lastLng = lng();
  lastPrediction = millis();
}

void predictPositions() {
  if (lastPrediction == 0) {
    lastPrediction = millis();
    return;
  }
  
  // Course prediction
  courseP = course() + (yawOriginal - yaw());

  // Calculate lat & long accel in m/s^2
  double rawAng = atan2(accelx(), accely()) + (90 - courseP);
  double mag = sqrt(accelx()*accelx() + accely()*accely());
  double accelY = sin(rawAng) * mag;
  double accelX = cos(rawAng) * mag;
  unsigned long currentTime = millis();

  // Integrate velocity
  double deltaT = ((double)(currentTime - lastPrediction)/1000);
  lastPrediction = currentTime;
  velX += accelX * deltaT;
  velY += accelY * deltaT;

  // Integrate position
  lngP += (velX * deltaT)/111111.0;
  latP += (velY * deltaT)/111111.0;
}

double predictedCourse() {
  return courseP;
}

double predictedLat() {
  return latP;
}

double predictedLng() {
  return lngP;
}
