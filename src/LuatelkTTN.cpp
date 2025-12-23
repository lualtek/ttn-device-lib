#include "LualtekTTN.h"
#include <EEPROM.h>


// --- Constructor ---
LualtekTTN::LualtekTTN(
  const char *appEui,
  const char *appKey,
  lorawan_class_t deviceClass,
  lualtek_dowlink_command_dutycycle_index_t dutyCycleIndex,
  TheThingsNetwork &ttn,
  Stream &debugStream
) :
  _appEui(appEui),
  _appKey(appKey),
  _deviceClass(deviceClass),
  _defaultDutyCycleIndex(dutyCycleIndex),
  _ttn(&ttn),
  _debugStream(&debugStream)
{
  _previousMillis = 0;
  if (isValidDutyCycleIndex(dutyCycleIndex)) {
      _uplinkInterval = DUTY_CYCLE_TABLE[dutyCycleIndex];
  } else {
      _uplinkInterval = DUTY_CYCLE_TABLE[MINUTES_15_COMMAND_INDEX];
  }
}

// --- Join Logic with ADR Sweep ---
bool LualtekTTN::join(JoinBehavior behavior, uint8_t retriesPerDr) {
  _debugStream->println(F("LoRaWAN Join requested (ADR Sweep Mode)"));

  while (true) {
    // Sweep from DR5 (SF7) down to DR0 (SF12)
    // Note: In EU868, DR5=SF7, DR0=SF12.
    for (int dr = 5; dr >= 0; dr--) {

      // Try 'retriesPerDr' times at this specific Data Rate
      for (int attempt = 1; attempt <= retriesPerDr; attempt++) {

        _debugStream->print(F("--> Join Attempt: DR"));
        _debugStream->print(dr);
        _debugStream->print(F(" (Try "));
        _debugStream->print(attempt);
        _debugStream->print(F("/"));
        _debugStream->print(retriesPerDr);
        _debugStream->println(F(")"));

        // Force the Data Rate before joining
        _ttn->setDR(dr);
        delay(500); // Brief pause for module to process serial

        // Attempt join with 1 retry (effectively 1 attempt + 1 retry by lib logic,
        // or just 1 attempt depending on lib version. We keep it short).
        // 4000ms wait window is usually enough for RX1/RX2 on Join.
        bool joined = _ttn->join(_appEui, _appKey, 1, 4000, _deviceClass);

        if (joined) {
          _debugStream->println(F("Successfully Joined!"));

          // --- Post Join Config ---
          // Enable ADR mechanism for future uplinks
          _ttn->setADR(true);

          resetSendInterval();

          // Visualize assigned address (Optional, helps debug)
          // _ttn->showStatus();

          return true;
        }

        // Random backoff between immediate retries to avoid collision storms
        // increasing slightly as we go
        unsigned long backoff = random(2000, 5000) + (1000 * (5 - dr));
        _debugStream->print(F("Failed. Waiting "));
        _debugStream->print(backoff);
        _debugStream->println(F("ms..."));
        systemWait(backoff);
      }
    }

    // --- Handling Sweep Failure ---
    if (behavior == JOIN_ONCE) {
      _debugStream->println(F("Err: Full join sweep failed. Giving up."));
      return false;
    }

    _debugStream->println(F("Err: Full join sweep failed. Deep sleep 2 mins..."));

    systemWait(60000);
  }
}

// --- Helpers & Standard Methods ---

bool LualtekTTN::isValidDutyCycleIndex(int index) {
  return index >= 0 && index < (int)(sizeof(DUTY_CYCLE_TABLE) / sizeof(DUTY_CYCLE_TABLE[0]));
}

void LualtekTTN::systemWait(unsigned long ms) {
  delay(ms);
}

unsigned long LualtekTTN::getUplinkInterval() {
  return _uplinkInterval;
}

void LualtekTTN::resetSendInterval() {
  _previousMillis = millis();
}

void LualtekTTN::handleChangeDutyCycle(int commandIndex) {
  if (!isValidDutyCycleIndex(commandIndex)) {
    _debugStream->println(F("Err: Invalid duty cycle index"));
    return;
  }

  _uplinkInterval = DUTY_CYCLE_TABLE[commandIndex];

  if (EEPROM.read(EEPROM_ADDRESS_DUTY_CYCLE_INDEX) != commandIndex) {
      EEPROM.update(EEPROM_ADDRESS_DUTY_CYCLE_INDEX, commandIndex);
      _debugStream->println(F("EEPROM Updated."));
  }
  _debugStream->print(F("Duty cycle set to (ms): "));
  _debugStream->println(_uplinkInterval);
}

bool LualtekTTN::canSendUplink() {
  if ((unsigned long)(millis() - _previousMillis) >= _uplinkInterval) {
    _previousMillis = millis();
    return true;
  }
  return false;
}

void LualtekTTN::onSendUplink(void (*callback)(int appPort)) {
  _onSendUplinkCallback = callback;
}

void LualtekTTN::setup() {
  int storedIndex = EEPROM.read(EEPROM_ADDRESS_DUTY_CYCLE_INDEX);
  handleChangeDutyCycle(isValidDutyCycleIndex(storedIndex) ? storedIndex : _defaultDutyCycleIndex);
  randomSeed(analogRead(0));
}

void LualtekTTN::onDownlinkReceived(const uint8_t *payload, size_t size, port_t port) {
  _debugStream->print(F("Downlink on port: "));
  _debugStream->println(port);

  switch(port) {
    case DOWNLINK_ACTION_CHANGE_INTERVAL_PORT:
      if (size > 0) handleChangeDutyCycle(payload[0]);
      break;
    case DOWNLINK_ACTION_REJOIN_PORT:
      _debugStream->println(F("Rejoin command."));
      join(JOIN_FOREVER);
      break;
    default:
      break;
  }
}
