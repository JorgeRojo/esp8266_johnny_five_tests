## Debug android remotely
- [Configure on-device developer options](https://developer.android.com/studio/debug/dev-options?hl=en-419)
- ``adb devices`` to get list of connected devices
- ``adb -s {device id/name} reverse tcp:8081 tcp:8081`` to link your device with DevTools
    - ``adb -s 0b08e9510299dd0f reverse tcp:8081 tcp:8081``
- ``adb shell input keyevent 82`` to open dev menu in the device


## Start error width react-native-iot-wifi
- to fix execute: ``rm -rf  node_modules/react-native-iot-wifi/example/``