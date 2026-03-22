#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include <WiFi.h>
#include "DHT.h"
#include <Update.h>   // Required for OTA compatibility
#include <time.h>


// ----------------- WiFi & Cloud -----------------
const char DEVICE_LOGIN_NAME[] = "24d31d3d-5d5a-4688-abc6-243693396a74";
const char DEVICE_KEY[] = "KufMfCqypNKUbPg4Kdfhi2SNC";
const char SSID[] ="aatish";
const char PASS[] ="9962034504";


// ----------------- Sensor Pins -----------------
#define DHTPIN 4
#define DHTTYPE DHT11
#define SOIL_PIN 34
#define GAS_PIN 35
#define LDR_PIN 33
#define AIRFLOW_PIN 32

// ----------------- Optional Actuator Pins -----------------
#define HUMIDIFIER_PIN 14
#define DEHUMIDIFIER_PIN 27
#define SPRINKLER_PIN 26
#define FAN_PIN 25
#define WINDOW_PIN 33

// ----------------- LED Alert Pins -----------------
#define TEMP_LED 12
#define HUMID_LED 13
#define SOIL_LED 2
#define GAS_LIGHT_AIR_LED 15

// ----------------- Thresholds -----------------
#define TEMP_MIN 22.0
#define TEMP_MAX 28.0
#define HUMID_MIN 55.0
#define HUMID_MAX 75.0
#define SOIL_DRY 2000
#define GAS_LIMIT 1800
#define LDR_LIMIT 1000
#define AIRFLOW_LIMIT 2000

// ----------------- Variables -----------------
float temperature;
float humidity;
int soilMoisture;
int gasLevel;
int lightLevel;
int airflow;
bool test;

DHT dht(DHTPIN, DHTTYPE);

void onTestChange();

void initProperties() {
  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.addProperty(test, READWRITE, ON_CHANGE, onTestChange);
  ArduinoCloud.addProperty(temperature, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(humidity, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(soilMoisture, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(gasLevel, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(lightLevel, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(airflow, READ, ON_CHANGE, NULL);
}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);

void setup() {
  Serial.begin(115200);
  dht.begin();

  // --- WiFi must be working before time sync ---
  Serial.println("Connecting to WiFi...");
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n✅ WiFi Connected");

  // --- Time Sync ---
  Serial.println("⏳ Synchronizing time...");
  configTime(19800, 0, "pool.ntp.org", "time.google.com", "time.nist.gov");
  
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("   Waiting for time sync...");
    delay(500);
  }
  Serial.println("✅ Time Synchronized!");

 


  pinMode(TEMP_LED, OUTPUT);
  pinMode(HUMID_LED, OUTPUT);
  pinMode(SOIL_LED, OUTPUT);
  pinMode(GAS_LIGHT_AIR_LED, OUTPUT);

  pinMode(HUMIDIFIER_PIN, OUTPUT);
  pinMode(DEHUMIDIFIER_PIN, OUTPUT);
  pinMode(SPRINKLER_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(WINDOW_PIN, OUTPUT);

  digitalWrite(TEMP_LED, LOW);
  digitalWrite(HUMID_LED, LOW);
  digitalWrite(SOIL_LED, LOW);
  digitalWrite(GAS_LIGHT_AIR_LED, LOW);

  digitalWrite(HUMIDIFIER_PIN, LOW);
  digitalWrite(DEHUMIDIFIER_PIN, LOW);
  digitalWrite(SPRINKLER_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(WINDOW_PIN, LOW);

  // --- Cloud Setup ---
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  Serial.println("🌐 Attempting Cloud Connection...");
}

void loop() {
  ArduinoCloud.update();
  readSensors();
  controlEnvironment();
  delay(2000);
}

void readSensors() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  soilMoisture = analogRead(SOIL_PIN);
  gasLevel = analogRead(GAS_PIN);
  lightLevel = analogRead(LDR_PIN);
  airflow = analogRead(AIRFLOW_PIN);

  Serial.print("Temp: "); Serial.print(temperature);
  Serial.print("  Humidity: "); Serial.print(humidity);
  Serial.print("  Soil: "); Serial.print(soilMoisture);
  Serial.print("  Gas: "); Serial.print(gasLevel);
  Serial.print("  Light: "); Serial.print(lightLevel);
  Serial.print("  Airflow: "); Serial.println(airflow);
}

void controlEnvironment() {
  
  // Temperature LED Logic
  if (temperature >= 22.0 && temperature <= 31.0) {
    digitalWrite(TEMP_LED, LOW);
  } else {
    digitalWrite(TEMP_LED, HIGH);
  }

  // Soil Moisture Logic
  if (soilMoisture > 4200) {
    digitalWrite(SOIL_LED, HIGH);
    digitalWrite(SPRINKLER_PIN, HIGH);
  } else {
    digitalWrite(SOIL_LED, LOW);
    digitalWrite(SPRINKLER_PIN, LOW);
  }

  // Gas Logic + Fan + Window
  if (gasLevel > 1900) {
    digitalWrite(GAS_LIGHT_AIR_LED, HIGH);
    digitalWrite(FAN_PIN, HIGH);
    digitalWrite(WINDOW_PIN, HIGH);
  } else {
    digitalWrite(GAS_LIGHT_AIR_LED, LOW);
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(WINDOW_PIN, LOW);
  }

  // Light → Humidity LED indicator (as requested)
  if (lightLevel > 1000) {
    digitalWrite(HUMID_LED, HIGH);
  } else {
    digitalWrite(HUMID_LED, LOW);
  }
}


void onTestChange() {
  Serial.print("Cloud test value changed to: ");
  Serial.println(test);
}
