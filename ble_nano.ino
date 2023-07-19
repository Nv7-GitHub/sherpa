#if defined(ARDUINO_ARCH_NRF52840)
#include <ArduinoBLE.h>

#define BLE_UUID(ind) ("f731ae25-" ind "-4677-9604-4a75debdaad0")

BLEService service(BLE_UUID("0000"));
BLECharacteristic gpsCharacteristic(BLE_UUID("0001"), BLERead | BLEWrite | BLENotify, sizeof(float) * 3);
BLEByteCharacteristic statusCharacteristic(BLE_UUID("0002"), BLERead | BLEWrite | BLENotify);

void setupBle() {  
  if (!BLE.begin()) {
    Serial.println("Failed to initialize BLE");
  }
  BLE.setLocalName("SherpaN");
  BLE.setDeviceName("SherpaN");
  BLE.setAdvertisedService(service);
  service.addCharacteristic(gpsCharacteristic);
  service.addCharacteristic(statusCharacteristic);
  statusCharacteristic.writeValue(1);
  float gps[3] = { 0, 0, 0 };
  gpsCharacteristic.writeValue(gps, sizeof(gps));
   
  BLE.addService(service);
  BLE.advertise();

  Serial.print("BLE ADDRESS: ");
  Serial.println(BLE.address());
  delay(1000);
    
}

int bleStatusVal;
int bleStatus() {
  return bleStatusVal;
}

float blePos[3]; // lat, lng, heading
float bleLat() {
  return blePos[0];
}

float bleLng() {
  return blePos[1];
}

float bleHeading() {
  return blePos[2];
}

bool hasPos = false;
bool hasPhonePos() {
  return hasPos;
}


bool bleRead() {
  if (!BLE.connected()) {
    return false;
  }

  int statusVal = statusCharacteristic.value();
  if (statusVal != 0) {
    bleStatusVal = statusVal;
    if (statusVal == 1) {
      onGpsWrite();
    }
  }

  return true;
}

void onGpsWrite() {
  float* data = (float*)gpsCharacteristic.value();
  if (data[0] == 0 && data[1] == 0 && data[2] == 0) {
    return;
  }
  blePos[0] = data[0];
  blePos[1] = data[1];
  blePos[2] = data[2];
  hasPos = true;
}

unsigned long lastBleWrite = millis();
const int bleWriteInterval = 250;

void bleWriteStatus(int statusV) {
  unsigned long currentTime = millis();
  if (currentTime - lastBleWrite < bleWriteInterval) {
    return;
  }
  lastBleWrite = currentTime;
  statusCharacteristic.writeValue(statusV);
}

void bleWriteOk() {
  unsigned long currentTime = millis();
  if (currentTime - lastBleWrite < bleWriteInterval) {
    return;
  }
  lastBleWrite = currentTime;
  statusCharacteristic.writeValue(BLE_STATUS_OK());
  float pos[3] = { lat(), lng(), course() };
  gpsCharacteristic.writeValue(pos, sizeof(pos));
}

#endif
