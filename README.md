# Lualtek device library for TTN + RN2483 + ATMega320u

## How to use it

When developing a firmware using Platformio just add the repository link to your `platformio.ini` file:

```ini
[env:your_env]
platform = atmelavr
board = leonardo
framework = arduino
lib_deps =
  https://github.com/lualtek/ttn-device-lib.git
  thethingsnetwork/TheThingsNetwork@^2.7.2
```
