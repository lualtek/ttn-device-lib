
#include <LualtekTTN.h>

const char *appEui = "";
const char *appKey = "";

#define loraSerial Serial1
#define debugSerial Serial

#define freqPlan TTN_FP_EU868
#define PORT_UPLINK 4

TheThingsNetwork ttn(loraSerial, debugSerial, freqPlan, 9);
LualtekTTN llt(appEui, appKey, CLASS_C, MINUTES_60_COMMAND_INDEX, ttn, debugSerial);

// Payload bytes to be sent as uplink
static uint8_t uplinkPayload[100];
int payloadSize = 0;

void sendUplink() {
  payloadSize = 0;
  uplinkPayload[payloadSize++] = highByte(0);
  uplinkPayload[payloadSize++] = lowByte(0);

  // Smart delay to avoid collision
  llt.delayMillis(random(1000, 5000));
  ttn.sendBytes(uplinkPayload, sizeof(uplinkPayload), PORT_UPLINK, false);
  llt.resetSendInterval();
}

void message(const uint8_t *payload, size_t size, port_t port) {
  llt.onDownlinkReceived(payload, size, port);
  sendUplink();
}

void setup() {
  loraSerial.begin(57600);
  debugSerial.begin(9600);

  // Wait a maximum of 10s for Serial Monitor
  while (!loraSerial && millis() < 10000);

  // TTN setup for downlink messages and ADR
  ttn.onMessage(message);
  ttn.setADR(true);
  ttn.showStatus();

  llt.setup();
  llt.join();
  sendUplink();
}

void loop() {
  // Check for received data.
  ttn.poll();

// Send uplink every X seconds
  if (llt.canSendUplink()) {
    sendUplink();
  }
}
