#include <Arduino.h>
#include <ArduinoJson.h>
#include <MitsubishiHeatpumpIR.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "env.h"

IRSenderBitBang irSender(3);
MitsubishiHeatpumpIR *heatpumpIR;
ESP8266WebServer server(80);
StaticJsonBuffer<56> jsonBuffer;
JsonObject& aircon = jsonBuffer.createObject();

void setup() {
  // Some comfortable defaults
  aircon["power"] = POWER_OFF;
  aircon["mode"] = MODE_COOL;
  aircon["temperature"] = 25;

  heatpumpIR = new MitsubishiMSYHeatpumpIR();
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  server.begin();
  server.on("/", handleRequest);
}

void loop() {
  server.handleClient();
}

void handleRequest() {
  if (server.hasArg("power")) {
    if (server.arg("power").toInt() == 1) {
      aircon["power"] = POWER_ON;
    } else if (server.arg("power").toInt() == 0) {
      aircon["power"] = POWER_OFF;
    }
  }

  if (server.hasArg("temperature")) {
    if (server.arg("temperature").toInt() >= 18 && server.arg("temperature").toInt() <= 30) {
      aircon["temperature"] = byte(server.arg("temperature").toInt());
    }
  }

  if (server.hasArg("mode")) {
    if (server.arg("mode").toInt() == 3) {
      aircon["mode"] = MODE_COOL;
    } else if (server.arg("mode").toInt() == 2) {
      aircon["mode"] = MODE_HEAT;
    }
  }

  if (server.hasArg("power") || server.hasArg("temperature") || server.hasArg("mode")) {
    heatpumpIR->send(irSender, aircon["power"], aircon["mode"], FAN_AUTO, aircon["temperature"], VDIR_AUTO, HDIR_AUTO);
  }

  char buffer[56];
  aircon.printTo(buffer, sizeof(buffer));
  server.send(200, "application/json", String(buffer));
}
