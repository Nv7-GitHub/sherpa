void setup() {
  Serial.begin(9600);
  setupMotors();
  setupMpu();
  setupGps();
  //setupBle();
  delay(5000);
}

void loop() {  
  // Read sensors
  int statusVal = 1;
  if (!mpuRead()) {
    Serial.println("MPU READ FAIL");
    statusVal = 2;
  }
  if (!gpsRead()) {
    Serial.println("GPS READ FAIL");
    statusVal = 3;
  }
  if (!bleRead()) {
    Serial.println("BLE NOT CONNECTED");
    return;
  }
  if (statusVal != 1) {
    bleWrite(0, 0, statusVal);
    return;
  }

  // Position estimation
  predictPositions();

  // BLE sending
  bleWrite(lat(), lng(), 1);

  // Drive
  if (hasPhonePos()) {
    move();
  }

  // Log
  /*Serial.print("yaw:");
  Serial.print(yaw());
  Serial.print(",pitch:");
  Serial.print(pitch());
  Serial.print(",roll:");
  Serial.print(roll());*/
  
  /*Serial.print(",accelx:");
  Serial.print(accelx());
  Serial.print(",accely:");
  Serial.print(accely());
  Serial.print(",accelz:");
  Serial.print(accelz());*/

  /*Serial.print(",latitude:");
  Serial.print(lat());
  Serial.print(",longitude:");
  Serial.print(lng());
  Serial.print(",altitude:");
  Serial.print(alt());
  Serial.print(",course:");
  Serial.print(course());
  Serial.print(",satCount:");
  Serial.print(satCount());*/

  /*Serial.print(",bleLat:");
  Serial.print(bleLat());
  Serial.print(",bleLng:");
  Serial.print(bleLng());
  Serial.print(",bleStatus:");
  Serial.print(bleStatus())*/;

  /*Serial.print(",predictedLat:");
  Serial.print(predictedLat()*111111.0);
  Serial.print(",predictedLng:");
  Serial.print(predictedLng()*111111.0);
  Serial.print(",predictedCourse:");
  Serial.print(predictedCourse());*/
  
  Serial.println("");
}
