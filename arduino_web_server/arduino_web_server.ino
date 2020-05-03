// Web server running on the Arduino.

#include <SoftwareSerial.h>
#include <Stepper.h>
#include "SR04.h"
#include <dht_nonblocking.h>

#include "credentials.h"

#define DHT_SENSOR_TYPE DHT_TYPE_11
#define BAUDRATE 9600
#define DEBUG true
#define STEPS 32
#define ULTRA_TRIG 4
#define ULTRA_ECHO 5
#define DHT 6
#define RELAY1 12
#define RELAY2 13


#define AT_OK "OK\r\n"

SoftwareSerial esp8266(2, 3);
DHT_nonblocking dht_sensor(DHT, DHT_SENSOR_TYPE);
Stepper stepper(STEPS, 8, 10, 9, 11);


SR04 sr04 = SR04(ULTRA_ECHO,ULTRA_TRIG);

void SendCommand(String command) {
  esp8266.print(command + "\r\n");
}


String AwaitResponse(String expected_suffix, const int timeout_ms) {
  long int deadline = millis() + timeout_ms;
  String response = "";
  while (millis() < deadline && !response.endsWith(expected_suffix)) {
    if (esp8266.available()) {
      response += esp8266.readString();
    }
  }
  if (DEBUG) {
    if (!response.endsWith(expected_suffix)) {
      Serial.println("TIMED OUT");
    }
    Serial.println("Got response -->" + response + "<--");
  }
  return response;
}

void SendHTTPResponse(const int connection_id,
                        String status_line,
                        String content) {
  String http_response = "HTTP/1.1 " + status_line + "\r\n";
  http_response += "Content-Type: text/html\r\n";
  http_response += "Connection: Closed\r\n";
  http_response += "data: " + content + "\r\n";

  char buffer[64];
  sprintf(buffer, "AT+CIPSEND=%d,%d", connection_id, http_response.length());
  SendCommand(buffer);
  AwaitResponse("> ", 2000);

  esp8266.print(http_response);
  Serial.println(http_response);
  AwaitResponse("SEND OK\r\n", 5000);

  sprintf(buffer, "AT+CIPCLOSE=%d", connection_id);
  SendCommand(buffer);
  AwaitResponse(AT_OK, 3000);
}

void InitWifiModule() {
  SendCommand("AT+RST");
  AwaitResponse(AT_OK, 4000);

  char buffer[50];
  sprintf(buffer, "AT+CWJAP=\"%s\",\"%s\"", WIFI_SSID, WIFI_PASS);
  SendCommand(buffer);
  AwaitResponse(AT_OK, 5000);

  SendCommand("AT+CWMODE=1");
  AwaitResponse(AT_OK, 1000);

  SendCommand("AT+CIFSR");
  AwaitResponse(AT_OK, 1000);

  SendCommand("AT+CIPMUX=1");
  AwaitResponse(AT_OK, 1000);

  SendCommand("AT+CIPSERVER=1,80");
  AwaitResponse(AT_OK, 5000);
}

void setup() {
  Serial.begin(BAUDRATE);
  esp8266.begin(BAUDRATE);

  pinMode(DHT, INPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);

  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);

  InitWifiModule();
  Serial.println("Ready for connection");


}

void loop() {

  char buffer[256];
  if (esp8266.available()) {
    if (esp8266.find("+IPD,")) {
      delay(1000);
      int connectionId = esp8266.read() - 48;
      Serial.print("Connection ID: ");
      Serial.println(connectionId);

      String httpRequest = connectionId + "";
      long int deadline = millis() + 2000;
      while (esp8266.available() || millis() < deadline) {
        // Note that the incoming request will be cut off at 64 bytes.
        httpRequest += esp8266.readString();
      }
      Serial.print("Incoming request: ");
      Serial.println("-->" + httpRequest + "<--");

      if (httpRequest.indexOf("led_on HTTP/1.1")!= -1 ) {
        Serial.println("led on");
        digitalWrite(RELAY2, LOW);
        SendHTTPResponse(connectionId, "200 OK", "success");
      }
      else if (httpRequest.indexOf("led_off HTTP/1.1")!= -1)  {
        digitalWrite(RELAY2, HIGH);
        SendHTTPResponse(connectionId, "200 OK", "success");
      }
      else if (httpRequest.indexOf("pump_on HTTP/1.1")!= -1)  {
        digitalWrite(RELAY1, LOW);
        SendHTTPResponse(connectionId, "200 OK", "success");
      }
      else if (httpRequest.indexOf("pump_off HTTP/1.1")!= -1)  {
        digitalWrite(RELAY2, HIGH);
        SendHTTPResponse(connectionId, "200 OK", "success");
      }
      else if (httpRequest.indexOf("sensors HTTP/1.1")!= -1)  {
        Serial.println("sensors");
        long dist = sr04.Distance();
        float temp = 0;
        float hum = 0;
        bool res = dht_sensor.measure( &temp, &hum );
        Serial.println(res);
        String ret;
        ret += temp;
        ret += ',';
        ret += hum;
        ret += ',';
        ret += dist;

        SendHTTPResponse(connectionId, "200 OK", ret);
      }
      else {
        Serial.println("ifs failed");
        SendHTTPResponse(connectionId, "200 OK", "fuck");
      }
    }
  }
}
