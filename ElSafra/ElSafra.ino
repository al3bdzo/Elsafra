#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include "DHT.h"

// ================== CONFIG ==================
#define DHTPIN 4
#define DHTTYPE DHT11

const char* ssid = "Elsafra";
const char* password = "12345678";

// Pins
int LDR1 = 32;
int LDR2 = 33;
int servopin = 18;
const int Analog_channel_pin = 36;

// ================== OBJECTS ==================
WebServer server(80);
Servo servo1;
DHT dht(DHTPIN, DHTTYPE);

// ================== STATE ==================
int currentServoAngle = 90;
bool manualControl = false;
int ldrOffset = 200; 

float temperature = 0;
float humidity = 0;
float voltage_value = 0.0;

// Timing
unsigned long lastSensorMillis = 0;
const unsigned long SENSOR_INTERVAL = 1000;

// Tracking parameters
int error = 400;        // dead zone (tune this)
float Kp = 0.002;      // proportional gain (tune this)

// ================== UTIL ==================
int smoothRead(int pin) {
  int sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += analogRead(pin);
    delay(2);
  }
  return sum / 5;
}

// ================== SENSOR UPDATE ==================
void updateSensors() {
  float newH = dht.readHumidity();
  float newT = dht.readTemperature();

  if (!isnan(newH)) humidity = newH;
  if (!isnan(newT)) temperature = newT;

  int ADC_VALUE = analogRead(Analog_channel_pin);
  voltage_value = ((ADC_VALUE * 3.3) / 4095.0) * 2.0;
}

// ================== TRACKING ==================
void autoAdjustServo() {
  int R1 = smoothRead(LDR1);
  int R2 = smoothRead(LDR2);

  float weight = 0.4; // Weight of R1 in the decision Making (it's always higher than R2 for some reason)

  int diff = (R1 * weight - R2) - ldrOffset;

  Serial.print("R1: "); Serial.print(R1);
  Serial.print(" R2: "); Serial.print(R2);
  Serial.print(" Diff: "); Serial.println(diff);

  if (abs(diff) < error) return;

  int adjustment = diff * Kp;

  if (adjustment == 0) {
    adjustment = (diff > 0) ? 1 : -1;
  }

  adjustment = constrain(adjustment, -3, 3);

  currentServoAngle -= adjustment;

  currentServoAngle = constrain(currentServoAngle, 0, 180);

  servo1.write(currentServoAngle);
}

// ================== API ==================
void handleStatus() {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  int R1 = smoothRead(LDR1);
  int R2 = smoothRead(LDR2);

  String json = "{";
  json += "\"temperature\":" + String(temperature, 1) + ",";
  json += "\"humidity\":" + String(humidity, 1) + ",";
  json += "\"voltage\":" + String(voltage_value, 2) + ",";
  json += "\"servo\":" + String(currentServoAngle) + ",";
  json += "\"ldr1\":" + String(R1) + ",";
  json += "\"ldr2\":" + String(R2) + ",";
  json += "\"manual\":" + String(manualControl ? 1 : 0);
  json += "}";

  server.send(200, "application/json", json);
}

// Unified control endpoint
void handleControl() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  
  if (server.hasArg("angle")) {
    int angle = constrain(server.arg("angle").toInt(), 0, 180);
    currentServoAngle = angle;
    servo1.write(currentServoAngle);
  }

  if (server.hasArg("manual")) {
    manualControl = server.arg("manual") == "1";
  }

  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleNotFound() {
  server.send(404, "application/json", "{\"error\":\"Not found\"}");
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  delay(1000);

  // WiFi AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));

  Serial.println("ESP32 AP Started");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  // API routes
  server.on("/status", handleStatus);
  server.on("/control", handleControl);
  server.onNotFound(handleNotFound);
  server.begin();

  // Hardware init
  servo1.attach(servopin);
  servo1.write(currentServoAngle);

  pinMode(LDR1, INPUT);
  pinMode(LDR2, INPUT);

  dht.begin();

  updateSensors();
}

// ================== LOOP ==================
void loop() {
  unsigned long now = millis();

  // Update sensors periodically
  if (now - lastSensorMillis >= SENSOR_INTERVAL) {
    lastSensorMillis = now;
    updateSensors();
  }

  // Auto tracking
  if (!manualControl) {
    autoAdjustServo();
  }

  // Handle API requests
  server.handleClient();
}