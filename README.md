# esp8266 
## configure johnny-fiveecho
 - follow the steps to stup esp8266 chip [esp8266-with-johnny-five](https://boneskull.com/how-to-use-an-esp8266-with-johnny-five/)

# React native
## dependencies
- [react-native-iot-wifi](https://www.npmjs.com/package/react-native-iot-wifi)

## Debug android remotely
- [Configure on-device developer options](https://developer.android.com/studio/debug/dev-options?hl=en-419)
- ``adb devices`` to get list of connected devices
- ``adb -s {device id/name} reverse tcp:8081 tcp:8081`` to link your device with DevTools
- ``adb shell input keyevent 82`` to open dev menu in the device