#include "LualtekTTN.h"
#include "DutyCycleHandler.h"
#if defined(__SAMD21G18A__) || defined(__SAMD21J18A__) || defined(__SAMD21E18A__)
// SAMD21 microcontroller detected
#include <FlashAsEEPROM.h>
#else
// Assume a different microcontroller, use EEPROM library
#include <EEPROM.h>
#endif

bool isDutyCycleIndex(unsigned int commandIndex) {
  return commandIndex >= 0 && commandIndex <= sizeof(dutyCycleCommandTable) - 1;
}

LualtekTTN::LualtekTTN(
  const char *appEui,
  const char *appKey,
  lorawan_class_t deviceClass,
  lualtek_dowlink_command_dutycycle_index_t dutyCycleIndex,
  TheThingsNetwork &ttn,
  Stream &debugStream
) {
  this->previousMillis = 0;
  this->defaultDutyCycleIndex = dutyCycleIndex;
  this->uplinkInterval = dutyCycleCommandTable[dutyCycleIndex];
  this->appEui = appEui;
  this->appKey = appKey;
  this->deviceClass = deviceClass;
  this->debugStream = &debugStream;
  this->ttn = &ttn;
}

unsigned long LualtekTTN::getUplinkInterval() {
  return this->uplinkInterval;
}

void LualtekTTN::resetSendInterval() {
  this->previousMillis = millis();
}

void LualtekTTN::delayMillis(unsigned long millisToWait) {
  unsigned long currentMillis = millis();
  while (millis() < currentMillis + millisToWait);
}

void LualtekTTN::handleChangeDutyCycle(int commandIndex) {
  if (!isDutyCycleIndex(commandIndex)) {
    this->debugStream->println("Invalid duty cycle index");
    this->debugStream->print("Duty cycle index: ");
    this->debugStream->println(commandIndex);
    return;
  }

  this->uplinkInterval = dutyCycleCommandTable[commandIndex];
  EEPROM.update(EEPROM_ADDRESS_DUTY_CYCLE_INDEX, commandIndex);
  #include "DutyCycleHandler.h"
#if defined(__SAMD21G18A__) || defined(__SAMD21J18A__) || defined(__SAMD21E18A__)
  // SAMD21 microcontroller detected
  EEPROM.commit();
#endif
  this->debugStream->println("Duty cycle changed");
  this->debugStream->print("Duty cycle: ");
  this->debugStream->println(this->uplinkInterval);
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

void LualtekTTN::join() {
  this->delayMillis(random(500, 8000));
  this->ttn->join(this->appEui, this->appKey, -1, 5000, this->deviceClass);
}

void LualtekTTN::onDownlinkReceived(const uint8_t *payload, size_t size, port_t port) {
  switch(port) {
    case DOWNLINK_ACTION_CHANGE_INTERVAL_PORT:
      this->debugStream->println("Received downlink for changing duty cycle");
      this->handleChangeDutyCycle(payload[0]);
      break;
    case DOWNLINK_ACTION_REJOIN_PORT:
      this->debugStream->println("Received downlink for rejoin. Rejoining...");
      this->join();
      break;
    default:
      break;
  }
}

void LualtekTTN::setup() {
  // Setup duty cycle from EEPROM if available or use default
  int currentDutyCycleIndex = EEPROM.read(EEPROM_ADDRESS_DUTY_CYCLE_INDEX);
  this->handleChangeDutyCycle(isDutyCycleIndex(currentDutyCycleIndex) ? currentDutyCycleIndex : this->defaultDutyCycleIndex);
}
