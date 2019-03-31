#include <Arduino.h> 
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h> 
#include <WebSocketsClient.h>
//https://github.com/Links2004/arduinoWebSockets
//https://github.com/Links2004/arduinoWebSockets/blob/master/examples/esp32/WebSocketClient/WebSocketClient.ino


WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

class WifiNotifier
{
  private:
    RGBLed _rgbled = RGBLed(0, 0, 0);
    Storage _storage = Storage(false);

    void _connect()
    {
    }

  public:
    bool on = false;

    WifiNotifier(RGBLed &rgbled, Storage &storage)
    {
        this->_rgbled = rgbled;
        this->_storage = storage;
    }

    void setup(std::string device_name)
    {
        this->_storage.load(false);
    }

    void start()
    {
        if (!this->_storage.data.wifi_ssid || !this->_storage.data.wifi_password)
        {
            this->on = false;
        }
        else if (!this->on)
        {
            //TODO: white wifi connection

            this->_rgbled.blink_green();
            this->on = true;
            Serial.println("WIFI -> START");
        }
    }

    void stop()
    {
        if (this->on)
        {
            this->on = false;
            Serial.println("WIFI -> STOP");
        }
    }

    void loop()
    {
    }

    void mpu_face_notify(char *face)
    {
        Serial.print("WIFI -> MPU FACE\t");
        Serial.println(face);
    }

    void battery_level_notify(float level)
    {
        Serial.print("WIFI -> BATTERY LEVEL\t");
        Serial.println(level);
    }
};
