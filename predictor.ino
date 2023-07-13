double latP;
double lngP;

// Course prediction
double courseP;
double yawOriginal;

// Position prediction
double velX = 0;
double velY = 0;
unsigned long lastPrediction = millis();

void gpsHasBeenUpdated() {
  yawOriginal = yaw();
  latP = lat();
  lngP = lng();
}

void predictPositions() {
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
  velX += accelX * deltaT;
  velY += accelY * deltaT;

  // Integrate position
  lngP += (velX * deltaT)/111111;
  latP += (velY * deltaT)/111111;
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
