
#ifndef _LUALTEKTTNLIB_H_
#define _LUALTEKTTNLIB_H_

#include <Arduino.h>
#include <TheThingsNetwork.h>

// For TTN compatibility
typedef uint8_t port_t;

#ifndef EEPROM_ADDRESS_DUTY_CYCLE_INDEX
  #define EEPROM_ADDRESS_DUTY_CYCLE_INDEX 10
#endif

enum lualtek_downlink_command_ports_t {
  DOWNLINK_ACTION_COMMAND_PORT = 1,
  DOWNLINK_ACTION_CHANGE_INTERVAL_PORT = 3,
  DOWNLINK_ACTION_REJOIN_PORT = 10
};

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

enum lualtek_dutycycle_ms_t {
  MINUTES_60_IN_MILLISECONDS = 3600000,
  MINUTES_40_IN_MILLISECONDS = 2400000,
  MINUTES_30_IN_MILLISECONDS = 1800000,
  MINUTES_20_IN_MILLISECONDS = 1200000,
  MINUTES_15_IN_MILLISECONDS = 900000,
  MINUTES_10_IN_MILLISECONDS = 600000,
  MINUTES_5_IN_MILLISECONDS = 300000,
  MINUTES_1_IN_MILLISECONDS = 60000
};

const lualtek_dutycycle_ms_t dutyCycleCommandTable[] PROGMEM = {
  MINUTES_60_IN_MILLISECONDS,
  MINUTES_40_IN_MILLISECONDS,
  MINUTES_30_IN_MILLISECONDS,
  MINUTES_20_IN_MILLISECONDS,
  MINUTES_15_IN_MILLISECONDS,
  MINUTES_10_IN_MILLISECONDS,
  MINUTES_5_IN_MILLISECONDS,
  MINUTES_1_IN_MILLISECONDS
};

class LualtekTTN {
  public:
    /* Please use one of the available lualtek_dowlink_command_dutycycle_index_t constants for dutyCycleIndex */
    LualtekTTN(
      const char *appEui,
      const char *appKey,
      lorawan_class_t deviceClass,
      lualtek_dowlink_command_dutycycle_index_t dutyCycleIndex,
      TheThingsNetwork &ttn,
      Stream &debugStream
    );
    void delayMillis(unsigned long millisToWait);
    /* Calculate if should send uplink based on duty cycle */
    bool canSendUplink();
    void resetSendInterval();
    /* Setup the device with common operations to be done, like setting the device duty cycle, class, region etc */
    void setup();

    void join();

    /**
     * @brief This method is called when the device needs to send an uplink message.
    */
    void onSendUplink(void (*cb)(int appPort));
    /**
     * @brief This method is called when a downlink message is received and after the message is processed.
     * The message will be processed by the library and will not trigger this callback when
     * the message PORT is one of the following (which are basically reserved for the library):
     * - DOWNLINK_ACTION_CHANGE_INTERVAL_PORT 3
     * - DOWNLINK_ACTION_REJOIN_PORT 10
    */
    void onDownlinkReceived(const uint8_t *payload, size_t size, port_t port);


    void handleChangeDutyCycle(int commandIndex);

  private:
    unsigned long previousMillis;
    unsigned long uplinkInterval;

    lualtek_dowlink_command_dutycycle_index_t defaultDutyCycleIndex;
    lorawan_class_t deviceClass;
    const char *appEui;
    const char *appKey;

    void (*onSendUplinkCallback)(int appPort);

    Stream *debugStream = NULL;
    TheThingsNetwork *ttn = NULL;
};
#endif
