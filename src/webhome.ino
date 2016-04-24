#include "IRremote2.h"
#include <UIPEthernet.h>

EthernetClient client;
IRsend hvacSend;
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE};
IPAddress ip(192,168,0,80);
EthernetServer server(1569);

String input;


void setup() {
  pinMode(2, OUTPUT);
  Ethernet.begin(mac, ip);
  server.begin();
}

void loop() {
  client = server.available();
  if (client) {
    if (client.available() > 0) {
      char thisChar = client.read();
      input += thisChar;
      handleMsg();

      if (thisChar == '\n' || thisChar == '\r') {
        input = "";
        client.flush();
      }
    }

    if (!client.connected()) {
      client.stop();
    }
  }
}

void handleMsg() {
  if (input == "AC OFF") {
    hvacSend.sendHvacMitsubishi(HVAC_COLD, 25, FAN_SPEED_AUTO, VANNE_AUTO_MOVE, true);
    client.println("AC turning off.");
  }

  if (input == "AC ON") {
    hvacSend.sendHvacMitsubishi(HVAC_COLD, 25, FAN_SPEED_AUTO, VANNE_AUTO_MOVE, false);
    client.println("Ac turning on.");
  }
}
