#include <Arduino.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <MitsubishiHeatpumpIR.h>
#include <PubSubClient.h>
#include <ThingSpeak.h>
#include "env.h"

#define DHTPIN 2
#define DHTTYPE DHT22

DHT                  dht(DHTPIN, DHTTYPE);
DynamicJsonBuffer    jsonBuffer;
IRSenderBitBang      irSender(3);
MitsubishiHeatpumpIR *heatpumpIR;
WiFiClient           espClient;
PubSubClient         client(espClient);

JsonObject& aircon  = jsonBuffer.createObject();

const int tempPoll = 300 * 1000;
long lastPoll      = 0;

void setupWifi() {
  WiFi.disconnect(true);
  delay(500);

  WiFi.begin(SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void mqttConnect() {
  while (!client.connected()) {
    if (client.connect(WiFi.macAddress().c_str(), MQTT_USER, MQTT_PASS)) {
      client.subscribe("aircon");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  // Some comfortable defaults
  aircon["power"]       = POWER_OFF;
  aircon["mode"]        = MODE_COOL;
  aircon["temperature"] = 25;

  heatpumpIR = new MitsubishiMSYHeatpumpIR();

  setupWifi();
  dht.begin();

  ThingSpeak.begin(espClient);

  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(handleMsg);
  mqttConnect();

  readAmbient();
}

void loop() {
  if (!client.connected()) {
    mqttConnect();
  }

  client.loop();

  if (millis() >= lastPoll + tempPoll) {
    lastPoll = millis();
    readAmbient();
  }

  delay(100);
}

void readAmbient() {
  float h  = dht.readHumidity();
  float t  = dht.readTemperature();
  float hi = dht.computeHeatIndex(t, h, false);

  DynamicJsonBuffer ambientBuffer;
  JsonObject& ambient = ambientBuffer.createObject();

  ambient["humidity"]    = h;
  ambient["temperature"] = t;
  ambient["index"]       = hi;

  char buffer[80];
  ambient.printTo(buffer, sizeof(buffer));
  client.publish("ambient", buffer);

  ThingSpeak.setField(1, hi);
  ThingSpeak.setField(2, t);
  ThingSpeak.setField(3, h);
  ThingSpeak.writeFields(TS_CHANNEL_ID, TS_API_KEY);
}

void handleMsg(char* topic, byte* payload, unsigned int length) {
  char req[80];
  for (int i = 0; i < length; i++) {
    req[i] = (char)payload[i];
  }

  DynamicJsonBuffer reqBuffer;
  JsonObject& reqJson = reqBuffer.parseObject(req);

  if (!reqJson.success()) {
    return;
  }

	aircon["mode"]        = reqJson["mode"] ? reqJson["mode"] : aircon["mode"];
	aircon["power"]       = reqJson["power"] ? reqJson["power"] : aircon["power"];
	aircon["temperature"] = reqJson["temperature"] ? reqJson["temperature"] : aircon["temperature"];

	heatpumpIR->send(irSender, aircon["power"], aircon["mode"], FAN_AUTO, aircon["temperature"], VDIR_AUTO, HDIR_AUTO);
}

