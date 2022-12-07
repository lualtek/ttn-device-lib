#include "LualtekTTN.h"

LualtekTTN::LualtekTTN(
  unsigned long dutyCycleMs,
  lorawan_class_t deviceClass,
  ttn_fp_t deviceRegion,
  Stream &debugStream,
  bool debugEnabled
) {
  this->previousMillis = 0;
  this->dutyCycleMs = {
    MINUTES_60_IN_MILLISECONDS,
    MINUTES_40_IN_MILLISECONDS,
    MINUTES_30_IN_MILLISECONDS,
    MINUTES_20_IN_MILLISECONDS,
    MINUTES_15_IN_MILLISECONDS,
    MINUTES_10_IN_MILLISECONDS,
    MINUTES_5_IN_MILLISECONDS,
    dutyCycleMs
  };

  this->deviceClass = deviceClass;
  this->deviceRegion = deviceRegion;
  this->uplinkInterval = dutyCycleMs;
  this->debugStream = &debugStream;
  this->debugEnabled = debugEnabled;
}

void LualtekTTN::debugPrint(const char *message) {
  if (this->debugEnabled && this->debugStream != NULL) {
    this->debugStream->print(message);
  }
}

void LualtekTTN::debugPrintln(const char *message) {
  if (this->debugEnabled && this->debugStream != NULL) {
    this->debugStream->println(message);
  }
}

void LualtekTTN::debugPrintln(int message) {
  if (this->debugEnabled && this->debugStream != NULL) {
    this->debugStream->println(message);
  }
}

void LualtekTTN::delayMillis(unsigned long millisToWait) {
  unsigned long currentMillis = millis();
  while (millis() < currentMillis + millisToWait);
}

void LualtekTTN::handleChangeDutyCycle(int commandIndex) {
  int dutyCycleIndexAssinged = -1;

  switch (commandIndex) {
    case MINUTES_60_COMMAND_INDEX:
      this->uplinkInterval = this->dutyCycleMs.minutes60;
      dutyCycleIndexAssinged = MINUTES_60_COMMAND_INDEX;
      break;
    case MINUTES_40_COMMAND_INDEX:
      this->uplinkInterval = this->dutyCycleMs.minutes40;
      dutyCycleIndexAssinged = MINUTES_40_COMMAND_INDEX;
      break;
    case MINUTES_30_COMMAND_INDEX:
      this->uplinkInterval = this->dutyCycleMs.minutes30;
      dutyCycleIndexAssinged = MINUTES_30_COMMAND_INDEX;
      break;
    case MINUTES_20_COMMAND_INDEX:
      this->uplinkInterval = this->dutyCycleMs.minutes20;
      dutyCycleIndexAssinged = MINUTES_20_COMMAND_INDEX;
      break;
    case MINUTES_15_COMMAND_INDEX:
      this->uplinkInterval = this->dutyCycleMs.minutes15;
      dutyCycleIndexAssinged = MINUTES_15_COMMAND_INDEX;
      break;
    case MINUTES_10_COMMAND_INDEX:
      this->uplinkInterval = this->dutyCycleMs.minutes10;
      dutyCycleIndexAssinged = MINUTES_10_COMMAND_INDEX;
      break;
    case MINUTES_5_COMMAND_INDEX:
      this->uplinkInterval = this->dutyCycleMs.minutes5;
      dutyCycleIndexAssinged = MINUTES_5_COMMAND_INDEX;
      break;
    case MINUTES_DEFAULT_COMMAND_INDEX:
      this->uplinkInterval = this->dutyCycleMs.minutesDefault;
      dutyCycleIndexAssinged = MINUTES_DEFAULT_COMMAND_INDEX;
      break;
    default:
      break;
  }

  if (dutyCycleIndexAssinged != -1) {
    this->debugPrintln("Duty cycle changed");
    this->debugPrint("Duty cycle: ");
    this->debugPrintln(this->uplinkInterval);

    EEPROM.update(EEPROM_ADDRESS_DUTY_CYCLE_INDEX, dutyCycleIndexAssinged);
  }
}

bool LualtekTTN::canSendUplink() {
  unsigned long currentMillis = millis();

  if (currentMillis - this->previousMillis >= this->uplinkInterval) {
    this->previousMillis = currentMillis;
    return true;
  }

  return false;
}

void LualtekTTN::onSendUplink(void (*callback)(int appPort)) {
  this->onSendUplinkCallback = callback;
}

void LualtekTTN::onJoin(void (*callback)(void)) {
  this->onJoinCallback = callback;
}

void LualtekTTN::onDownlinkReceived(const uint8_t *payload, size_t size, port_t port) {
  switch(port) {
    case DOWNLINK_ACTION_CHANGE_INTERVAL_PORT:
      this->debugPrintln("Received downlink for changing duty cycle");
      this->handleChangeDutyCycle(payload[0]);
      break;
    case DOWNLINK_ACTION_REJOIN_PORT:
      this->debugPrintln("Received downlink for rejoin. Rejoining...");
      this->onJoinCallback();
      break;
    default:
      break;
  }
}

void LualtekTTN::setup() {
  // Setup duty cycle from EEPROM if available or use default
  int currentDutyCycleIndex = EEPROM.read(EEPROM_ADDRESS_DUTY_CYCLE_INDEX);
  if (currentDutyCycleIndex >= MINUTES_60_COMMAND_INDEX && currentDutyCycleIndex <= MINUTES_DEFAULT_COMMAND_INDEX) {
    this->handleChangeDutyCycle(currentDutyCycleIndex);
  } else {
    this->handleChangeDutyCycle(MINUTES_DEFAULT_COMMAND_INDEX);
  }
}
