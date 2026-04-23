#ifndef _LUALTEKTTNLIB_H_
#define _LUALTEKTTNLIB_H_

#include <Arduino.h>
#include <TheThingsNetwork.h>

// For TTN compatibility
typedef uint8_t port_t;

#ifndef EEPROM_ADDRESS_DUTY_CYCLE_INDEX
  #define EEPROM_ADDRESS_DUTY_CYCLE_INDEX 10
#endif

// --- Enums ---
enum JoinBehavior {
  JOIN_ONCE,
  JOIN_FOREVER
};

enum lualtek_downlink_command_ports_t {
  DOWNLINK_ACTION_COMMAND_PORT = 1,
  DOWNLINK_ACTION_CHANGE_INTERVAL_PORT = 3,
  DOWNLINK_ACTION_REJOIN_PORT = 10
};

// ... [Keep your existing Enums for Duty Cycle here] ...
enum lualtek_dowlink_command_dutycycle_index_t {
  MINUTES_60_COMMAND_INDEX = 0,
  MINUTES_40_COMMAND_INDEX = 1,
  MINUTES_30_COMMAND_INDEX = 2,
  MINUTES_20_COMMAND_INDEX = 3,
  MINUTES_15_COMMAND_INDEX = 4,
  MINUTES_10_COMMAND_INDEX = 5,
  MINUTES_5_COMMAND_INDEX = 6,
  MINUTES_1_COMMAND_INDEX = 7
};

const unsigned long DUTY_CYCLE_TABLE[] = {
  3600000, 2400000, 1800000, 1200000, 900000, 600000, 300000, 60000
};

class LualtekTTN {
  public:
    LualtekTTN(
      const char *appEui,
      const char *appKey,
      lorawan_class_t deviceClass,
      lualtek_dowlink_command_dutycycle_index_t dutyCycleIndex,
      TheThingsNetwork &ttn,
      Stream &debugStream
    );

    void setup();

    /**
     * @brief Performs a robust Join Sequence with ADR Sweep.
     * Sweeps from DR5 (SF7) down to DR0 (SF12).
     * * @param behavior JOIN_ONCE or JOIN_FOREVER
     * @param retriesPerDr How many times to try each Data Rate before lowering it (Default 3)
     */
    bool join(JoinBehavior behavior = JOIN_FOREVER, uint8_t retriesPerDr = 3);

    bool canSendUplink();
    void resetSendInterval();
    void onSendUplink(void (*cb)(int appPort));
    void onDownlinkReceived(const uint8_t *payload, size_t size, port_t port);
    void handleChangeDutyCycle(int commandIndex);
    unsigned long getUplinkInterval();

  private:
    unsigned long _uplinkInterval;
    unsigned long _previousMillis;

    lualtek_dowlink_command_dutycycle_index_t _defaultDutyCycleIndex;
    lorawan_class_t _deviceClass;
    const char *_appEui;
    const char *_appKey;

    void (*_onSendUplinkCallback)(int appPort) = NULL;

    Stream *_debugStream = NULL;
    TheThingsNetwork *_ttn = NULL;

    void systemWait(unsigned long ms);
    bool isValidDutyCycleIndex(int index);
};

#endif
