//https://github.com/LilyGO/ESP32-MINI-32-V2.0
//https://sparks.gogo.co.nz/ch340.html
#include "Arduino.h"
#include "./Storage.h"
#include "./RGBLed.h"
#include "./Bluetooth.h"
#include "./Orientation.h"
#include "./Battery.h"
#include "./WifiNotifier.h"

#define PIN_BATTERY A0
#define PIN_LED_RED 23
#define PIN_LED_GREEN 18
#define PIN_LED_BLUE 19

RGBLed rgbled = RGBLed(PIN_LED_RED, PIN_LED_GREEN, PIN_LED_BLUE);
Storage storage = Storage(false);
Battery battery = Battery(PIN_BATTERY);
Orientation orientation = Orientation(rgbled, storage);
WifiNotifier wifi = WifiNotifier(rgbled, storage);
Bluetooth bluetooth = Bluetooth(rgbled, storage);


void wifi_battery_level_notify(float level)
{
    wifi.battery_level_notify(level);
}

void wifi_mpu_face_notify(char *face)
{
    wifi.mpu_face_notify(face);
}

void wifi_connect(const char *ssid, const char *pass)
{
    wifi.connect(ssid, pass);
}

std::string get_polytt_id()
{
    uint64_t chipid = ESP.getEfuseMac();
    uint16_t chip = (uint16_t)(chipid >> 32);
    char uid[23];
    snprintf(uid, 23, "POLYTTE-%04X%08X", chip, (uint32_t)chipid);
    return std::string(uid);
}

std::vector<String> get_wifi_list()
{
    return wifi.get_wifi_list();
}

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        continue;
        delay(500);
    }

    delay(500);

    Serial.println();
    Serial.println("************** POLYTT SETUP **************");
    Serial.println();

    std::string uid = get_polytt_id();
    Serial.println(uid.c_str());

    battery.start();
    rgbled.start();
    orientation.start();
    bluetooth.setup(uid);
    wifi.setup(uid);

    battery.on_level_change(wifi_battery_level_notify);
    orientation.on_face_change(wifi_mpu_face_notify);
    bluetooth.on_wifi_data(wifi_connect);
    bluetooth.on_connect(get_wifi_list);

    delay(500);
}

void loop()
{
    // Battery --------------------------------
    float batteryLevel = battery.loop();
     //Serial.println(batteryLevel);
    if (batteryLevel <= 30)
    {
        rgbled.yellow();
    }
    if (batteryLevel <= 10)
    {
        rgbled.red();
    }

    // orientation ----------------------------
    if (batteryLevel > 10)
    {
        orientation.loop();
    }

    if (orientation.state == 4)
    {
        wifi.loop();

        if (!wifi.on)
        {
            bluetooth.start(); 
        }
        else
        {
            bluetooth.stop();
        }
        
        bluetooth.loop();

    }

    rgbled.loop();
    delay(100);
}
