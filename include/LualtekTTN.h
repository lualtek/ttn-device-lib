
#ifndef _LUALTEKTTNLIB_H_
#define _LUALTEKTTNLIB_H_

#include "EEPROM.h"
#include <Arduino.h>
#include <TheThingsNetwork.h>
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD)
#include <avr/pgmspace.h>
#else
#include <pgmspace.h>
#endif

// For TTN compatibility
typedef uint8_t port_t;

#ifndef EEPROM_ADDRESS_DUTY_CYCLE_INDEX
  #define EEPROM_ADDRESS_DUTY_CYCLE_INDEX 10
#endif

#define DOWNLINK_ACTION_COMMAND_PORT 1
#define DOWNLINK_ACTION_CHANGE_INTERVAL_PORT 3
#define DOWNLINK_ACTION_CHANGE_STEPPER_TIME_PORT 4
#define DOWNLINK_ACTION_REJOIN_PORT 10

#define MINUTES_60_IN_MILLISECONDS 3600000
#define MINUTES_40_IN_MILLISECONDS 2400000
#define MINUTES_30_IN_MILLISECONDS 1800000
#define MINUTES_20_IN_MILLISECONDS 1200000
#define MINUTES_15_IN_MILLISECONDS 900000
#define MINUTES_10_IN_MILLISECONDS 600000
#define MINUTES_5_IN_MILLISECONDS 300000

#define MINUTES_60_COMMAND_INDEX 0
#define MINUTES_40_COMMAND_INDEX 1
#define MINUTES_30_COMMAND_INDEX 2
#define MINUTES_20_COMMAND_INDEX 3
#define MINUTES_15_COMMAND_INDEX 4
#define MINUTES_10_COMMAND_INDEX 5
#define MINUTES_5_COMMAND_INDEX 6
#define MINUTES_DEFAULT_COMMAND_INDEX 7

struct DutyCycleMs {
  unsigned long minutes60;
  unsigned long minutes40;
  unsigned long minutes30;
  unsigned long minutes20;
  unsigned long minutes15;
  unsigned long minutes10;
  unsigned long minutes5;
  unsigned long minutesDefault;
};

class LualtekTTN {
  public:
    /* Please use one of the available MINUTES_X_IN_MILLISECONDS constants for dutyCycleMs */
    LualtekTTN(unsigned long dutyCycleMs, Stream &debugStream, bool debugEnabled);
    void delayMillis(unsigned long millisToWait);
    /* Calculate if should send uplink based on duty cycle */
    bool canSendUplink();
    void resetSendInterval();
    /* Setup the device with common operations to be done, like setting the device duty cycle, class, region etc */
    void setupAndJoin();

    void onJoin(void (*cb)(void));

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
    struct DutyCycleMs dutyCycleMs;

    void debugPrint(const String &s);
    void debugPrint(const char[]);
    void debugPrintln(const String &s);
    void debugPrintln(const char[]);
    void debugPrintln(int i);

    void (*onSendUplinkCallback)(int appPort);
    void (*onJoinCallback)(void);

    Stream *debugStream = NULL;
    bool debugEnabled = false;
};
#endif
