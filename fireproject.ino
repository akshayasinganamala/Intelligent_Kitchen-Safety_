#define BLYNK_TEMPLATE_ID "TMPL3AKouUyjU"
#define BLYNK_TEMPLATE_NAME "Kitchen Safety"
#define BLYNK_AUTH_TOKEN "uKMcdngBRz2pxLuNaxiSiQS5aMeRxbW6"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// WiFi
char ssid[] = "Akshaya";
char pass[] = "23062006";

// Pins
#define ONE_WIRE_BUS 4
#define GAS_PIN 34
#define PIR_PIN 5
#define BUZZER_PIN 18
#define LED_PIN 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Thresholds
float tempThreshold = 45.0;
float maxTemp = 60.0;
int gasThreshold = 300;

// Previous values
float prevTemp = 0;
int prevGas = 0;

// PIR timing
unsigned long lastMotionTime = 0;
const unsigned long noPersonDelay = 30000;

// Notification cooldown
unsigned long lastAlertTime = -30000;
const unsigned long alertCooldown = 30000;

// TEST MODE (set false after testing)
bool testMode = true;

// Notification function
void sendNotification(String msg) {
  if (millis() - lastAlertTime > alertCooldown) {
    Serial.println("📩 Sending: " + msg);
    Blynk.logEvent("danger_alert", msg);
    lastAlertTime = millis();
  }
}

int predict(float temp, float gas, float dTemp, float dGas, bool person) {

  if (temp > 55 && dGas > 80) {
    if (!person) return 3;
    else return 2;
  }

  if (temp < 50 && dGas < 30) {
    return 0;
  }

  return 1;
}

void setup() {
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  sensors.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting WiFi...");
    WiFi.begin(ssid, pass);
  }

  if (!Blynk.connected()) {
    Serial.println("Reconnecting Blynk...");
    Blynk.connect();
  }

  Blynk.run();

  // Temperature
  sensors.requestTemperatures();
  float temperature = sensors.getTempCByIndex(0);

  // Gas
  int gasValue = analogRead(GAS_PIN);

  // PIR
  int pirState = digitalRead(PIR_PIN);

  if (pirState == 1) {
    lastMotionTime = millis();
  }

  bool personPresent = (millis() - lastMotionTime) < noPersonDelay;

  float tempChange = abs(temperature - prevTemp);
  int gasChange = abs(gasValue - prevGas);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  // Debug
  Serial.print("Temp: "); Serial.print(temperature);
  Serial.print(" | Gas: "); Serial.print(gasValue);
  Serial.print(" | Person: "); Serial.print(personPresent);
  Serial.print(" | dT: "); Serial.print(tempChange);
  Serial.print(" | dG: "); Serial.println(gasChange);

  // CRITICAL (or test mode)
  if (testMode || temperature > maxTemp || gasValue > gasThreshold) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    sendNotification("CRITICAL: High Temp or Gas!");
  }

  // Cooking logic
  if (temperature > tempThreshold) {

    int result = predict(temperature, gasValue, tempChange, gasChange, personPresent);

    switch (result) {

      case 0:
        Serial.println("Safe");
        break;

      case 1:
        Serial.println("Normal Cooking");
        break;

      case 2:
        Serial.println("Warning");
        digitalWrite(LED_PIN, HIGH);
        break;

      case 3:
        Serial.println("DANGER: Unattended Cooking");
        digitalWrite(BUZZER_PIN, HIGH);
        digitalWrite(LED_PIN, HIGH);
        sendNotification("ALERT! Unattended Cooking!");
        break;
    }
  }

  prevTemp = temperature;
  prevGas = gasValue;

  delay(2000);
}
