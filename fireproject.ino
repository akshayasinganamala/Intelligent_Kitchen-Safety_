#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <OneWire.h>
#include <DallasTemperature.h>

/**************** WIFI ****************/
const char* ssid = "WIFI_NAME";
const char* password = "WIFI_PASSWORD";

/**************** FLASK SERVER ****************/
// Replace with laptop IP
const char* serverName =
"http://192.168.1.5:5000";

/**************** PIN CONNECTIONS ****************/
#define ONE_WIRE_BUS 4
#define GAS_PIN 34
#define PIR_PIN 5
#define BUZZER_PIN 18

/**************** DS18B20 ****************/
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {

  Serial.begin(115200);

  /******** PIN MODES ********/
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);

  /******** START TEMPERATURE SENSOR ********/
  sensors.begin();

  /******** WIFI CONNECT ********/
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  /**************** READ TEMPERATURE ****************/
  sensors.requestTemperatures();

  float temp =
    sensors.getTempCByIndex(0);

  /**************** READ GAS SENSOR ****************/
  int gas =
    analogRead(GAS_PIN);

  /**************** READ PIR SENSOR ****************/
  int person =
    digitalRead(PIR_PIN);

  /**************** PRINT VALUES ****************/
  Serial.println("---------------");

  Serial.print("Temperature: ");
  Serial.println(temp);

  Serial.print("Gas Value: ");
  Serial.println(gas);

  Serial.print("Person: ");
  Serial.println(person);

  /**************** BUZZER LOGIC ****************/
  // Simple local alert

  if (temp > 40 || gas > 1000) {

    digitalWrite(BUZZER_PIN, HIGH);

    Serial.println("BUZZER ON");
  }

  else {

    digitalWrite(BUZZER_PIN, LOW);

    Serial.println("BUZZER OFF");
  }

  /**************** SEND DATA TO FLASK ****************/
  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    http.begin(serverName);

    http.addHeader(
      "Content-Type",
      "application/json"
    );

    /**************** JSON ****************/
    StaticJsonDocument<200> doc;

    doc["temp"] = temp;
    doc["gas"] = gas;
    doc["person"] = person;

    String jsonData;

    serializeJson(doc, jsonData);

    /**************** POST REQUEST ****************/
    int httpResponseCode =
      http.POST(jsonData);

    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0) {

      String response =
        http.getString();

      Serial.println("Server Response:");
      Serial.println(response);
    }

    else {

      Serial.println("Error Sending Data");
    }

    http.end();
  }

  else {

    Serial.println("WiFi Disconnected");
  }

  delay(2000);
}
