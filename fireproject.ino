#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <OneWire.h>
#include <DallasTemperature.h>

/************ WIFI ************/
const char* ssid = "WIFI_NAME";
const char* password = "WIFI_PASSWORD";

/************ FLASK SERVER ************/
const char* serverName =
"http://192.168.1.5:5000";

/************ DS18B20 ************/
#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

/************ MQ SENSOR ************/
#define MQ_PIN 34

/************ PIR SENSOR ************/
#define PIR_PIN 27

void setup() {

  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);

  sensors.begin();

  /******** WIFI CONNECT ********/
  WiFi.begin(ssid, password);

  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
}

void loop() {

  /******** TEMPERATURE ********/
  sensors.requestTemperatures();

  float temp =
    sensors.getTempCByIndex(0);

  /******** GAS ********/
  int gas =
    analogRead(MQ_PIN);

  /******** PERSON ********/
  int person =
    digitalRead(PIR_PIN);

  /******** SERIAL PRINT ********/
  Serial.println("-----------");

  Serial.print("Temp: ");
  Serial.println(temp);

  Serial.print("Gas: ");
  Serial.println(gas);

  Serial.print("Person: ");
  Serial.println(person);

  /******** SEND TO FLASK ********/
  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    http.begin(serverName);

    http.addHeader(
      "Content-Type",
      "application/json"
    );

    /******** JSON ********/
    StaticJsonDocument<200> doc;

    doc["temp"] = temp;
    doc["gas"] = gas;
    doc["person"] = person;

    String jsonData;

    serializeJson(doc, jsonData);

    /******** POST ********/
    int httpResponseCode =
      http.POST(jsonData);

    Serial.print("HTTP Response: ");

    Serial.println(httpResponseCode);

    http.end();
  }

  else {
    Serial.println("WiFi Disconnected");
  }

  delay(2000);
}
