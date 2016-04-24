#include <UIPEthernet.h>
#include <MitsubishiHeatpumpIR.h>

EthernetClient client;
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE};
IPAddress ip(192,168,0,80);
EthernetServer server(1569);
IRSenderPWM irSender(3);
MitsubishiHeatpumpIR *heatpumpIR;

String input;


void setup() {
  heatpumpIR = new MitsubishiMSYHeatpumpIR();
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
    heatpumpIR->send(irSender, POWER_OFF, MODE_COOL, FAN_AUTO, 25, VDIR_AUTO, HDIR_AUTO);
    client.println("AC turning off.");
  }

  if (input == "AC ON") {
    heatpumpIR->send(irSender, POWER_ON, MODE_COOL, FAN_AUTO, 25, VDIR_AUTO, HDIR_AUTO);
    client.println("Ac turning on.");
  }
}
