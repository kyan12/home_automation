// Web server running on the Arduino.

#include <SoftwareSerial.h>

#include "credentials.h"

#define BAUDRATE 9600
#define DEBUG true

SoftwareSerial esp8266(2, 3);

String SendData(String command, const int timeout) {
  String response = "";
  esp8266.print(command);
  long int deadline = millis() + timeout;
  while (millis() < deadline && !response.endsWith("OK\r\n")) {
    if (esp8266.available()) {
      response += esp8266.readString();
    }
  }
  if (DEBUG) {
    Serial.println(response);      
  }
  return response;
}

void InitWifiModule() {      
  SendData("AT+RST\r\n", 2000);
  char buffer[50];
  sprintf(buffer, "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASS);
  SendData(buffer, 10000);
  SendData("AT+CWMODE=1\r\n", 5000);
  if (DEBUG) {
    SendData("AT+CIFSR\r\n", 5000);
  }
  SendData("AT+CIPMUX=1\r\n", 5000);
  SendData("AT+CIPSERVER=1,80\r\n", 5000);
}

void setup() {  
  Serial.begin(BAUDRATE);
  esp8266.begin(BAUDRATE);
  
  InitWifiModule();
  Serial.println("Waiting for connection...");
}      

void loop() {
  char buffer[256];
  if (esp8266.available()) {      
    if (esp8266.find("+IPD,")) {
      delay(1000);
      int connectionId = esp8266.read() - 48;
      Serial.print("Connection ID: ");
      Serial.println(connectionId);

      sprintf(buffer, "AT+CIPSEND=%d,2\r\n", connectionId);
      SendData(buffer, 4000);
      delay(500);
      SendData("OK\r\n", 1000);

      sprintf(buffer, "AT+CIPCLOSE=%d\r\n", connectionId);
      SendData(buffer, 1000);
    }      
  }
}
