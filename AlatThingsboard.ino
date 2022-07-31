#include "ThingsBoard.h"
#include <WiFi.h>
#include <OneWire.h> 
#include <DallasTemperature.h>

#define WIFI_AP             "Wifiku"
#define WIFI_PASSWORD       "Arimbawa12345"

#define pinPhSensor 35
#define pinTurbo 26
#define pinTemper 33
#define RELAY_FAN_PIN 25

// See https://thingsboard.io/docs/getting-started-guides/helloworld/
// to understand how to obtain an access token
#define TOKEN               "XLh1oWGWeQbF4jVpNxvT"
#define THINGSBOARD_SERVER  "192.168.1.5"

// Baud rate for debug serial
#define SERIAL_DEBUG_BAUD   115200

OneWire oneWire(pinTemper);
DallasTemperature temper(&oneWire);
// Initialize ThingsBoard client
WiFiClient espClient;
// Initialize ThingsBoard instance
ThingsBoard tb(espClient);
// the Wifi radio's status

int status = WL_IDLE_STATUS;

int phSensorRaw = 0;
int banyak_kalibrasi = 10;
float phSensor = 0;

void setup() {
  // initialize serial for debugging
  Serial.begin(SERIAL_DEBUG_BAUD);
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  InitWiFi();
  pinMode(RELAY_FAN_PIN, OUTPUT);
  temper.begin();
}

float ph_calc(){
  int kalibrasi = 0;

  for(int i = 0; i < banyak_kalibrasi; i++){
    kalibrasi += analogRead(pinPhSensor);
    delay(10);
  }
  phSensor = 3.3 / 4095.0 * (kalibrasi / banyak_kalibrasi);
  return 7 + ((2.5 - phSensor)/ 0.18);;
}
int turbidity_calc(){
  int turbidity_val = analogRead(pinTurbo);
  int turbidity = map(turbidity_val, 0,2800,5,1);
  return turbidity;
}

void loop() {
  delay(1000);
  temper.requestTemperatures();
  ///tempr = temper.getTempCByIndex(0);
   int TEMP_THRESHOLD_UPPER = 26;
   int TEMP_THRESHOLD_LOWER = 26;
  
   if(temper.getTempCByIndex(0) > TEMP_THRESHOLD_UPPER){
    Serial.println("The fan is turned on");
    digitalWrite(RELAY_FAN_PIN, HIGH); // turn on
    delay(10);
  }
    else if(temper.getTempCByIndex(0) < TEMP_THRESHOLD_LOWER){
    Serial.println("The fan is turned off");
    digitalWrite(RELAY_FAN_PIN, LOW); // turn on
     delay(10);
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    reconnect();
  }

  if (!tb.connected()) {
    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      return;
    }
  }

  Serial.println("Sending data...");

  // Uploads new telemetry to ThingsBoard using MQTT.
  // See https://thingsboard.io/docs/reference/mqtt-api/#telemetry-upload-api
  // for more details

  tb.sendTelemetryFloat("ph", ph_calc());
  tb.sendTelemetryFloat("Suhu", temper.getTempCByIndex(0));
  tb.sendTelemetryFloat("Tds", turbidity_calc());

  tb.loop();
}

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

void reconnect() {
  // Loop until we're reconnected
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    WiFi.begin(WIFI_AP, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to AP");
  }
}
