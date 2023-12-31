#if defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_MEGA)

#include <ArduinoJson.h>

// Connect BLE to VCC, GND, RX2, TX2
// Send "CONN\n" to establish connection

// Status values
// 1: OK
// 2: IMU FAIL
// 3: GPS FAIL
/*
JSON DOCUMENT (both master and slave send this format)
{
  "lat": 1.23,
  "lng": 1.23,
  "heading": 1.23,
  "status": 1
}
 */

void setupBle() {
  Serial2.begin(9600);
  Serial.println("HELP ME");
  Serial.println(ble_cmd("AT+NAMESherpaM","Device Name: ")); // printout device name
  Serial.println(ble_cmd("AT+LADDR","Address: ")); // printout BLE address
  Serial.println(ble_cmd("AT+CHAR","Char UUID: ")); // printout character UUID
  Serial.println(ble_cmd("AT+VERSION","Version: ")); // module version  
  Serial.println(ble_cmd("AT+RESET","")); // reset BLE module
}

bool conn = false;
bool hasPos = false;

String msg_buff = "";
StaticJsonDocument<96> msgJson; // Size generated with https://arduinojson.org/v6/assistant/

String str_ii;
int ii_0 = 0;
String ble_cmd(String cmd_str,String desc_str){
  Serial.println("GOT CMD");
  str_ii = "";
  unsigned long t1 = millis();
  Serial2.println(cmd_str);
  while (true){
    char in_char = Serial2.read();
    //Serial.println(in_char);
    if (int(in_char)==-1 or int(in_char)==42){
      if ((millis()-t1)>2000){ // 2 second timeout
        return "Err";
      }
      continue;
    }
    if (in_char=='\n'){
      Serial.print("Bluetooth "+desc_str);
      Serial.println(str_ii.substring(0,str_ii.length()));
      return str_ii;
    }
    str_ii+=in_char;
  }
}

bool bleRead() {
  while (Serial2.available()) {
    char c = Serial2.read();
    msg_buff += c;
    if (c == '\n') { // End of message
      if (msg_buff == "CONN\n") { // Connection
        conn = true;
        msg_buff = "";
      } else { // Json value
        DeserializationError error = deserializeJson(msgJson, msg_buff);
        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
        }
        hasPos = true;
        msg_buff = "";
      }
    }
  }

  return conn;
}

unsigned long lastBleWrite = millis();
const long bleWriteInterval = 250;

void bleWriteStatus(int statusV) {
  bleWrite(0, 0, 0, statusV);
}

void bleWriteOk() {
  bleWrite(lat(), lng(), course(), BLE_STATUS_OK());
}

void bleWrite(float latV, float lngV, float heading, int statusV) {
  unsigned long currentTime = millis();
  if (currentTime - lastBleWrite <= bleWriteInterval) {
    return;
  }
  lastBleWrite = currentTime;
  
  msgJson["lat"] = latV;
  msgJson["lng"] = lngV;
  msgJson["heading"] = heading;
  msgJson["status"] = statusV;
  serializeJson(msgJson, Serial2);
  Serial2.println("");
}

float bleLat() {
  return msgJson["lat"];
}


float bleLng() {
  return msgJson["lng"];
}

float bleHeading() {
  return msgJson["heading"];
}

int bleStatus() {
  return msgJson["status"];
}

bool hasPhonePos() {
  return hasPos;
}

#endif
