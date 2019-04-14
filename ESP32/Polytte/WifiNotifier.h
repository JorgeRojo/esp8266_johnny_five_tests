#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <WebSocketsClient.h>
//https://github.com/Links2004/arduinoWebSockets
//https://github.com/Links2004/arduinoWebSockets/tree/master/examples/esp32

WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

class WifiNotifier
{
  private:
    RGBLed _rgbled = RGBLed(0, 0, 0);
    Storage _storage = Storage(false);
    char *wifi_ssid = "";
    char *wifi_password = "";

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

    void connect(const char *ssid, const char *pass)
    {
        if (!ssid || !pass)
        {
            this->on = false;
        }

        if (!this->on)
        {
            WiFiMulti.addAP(ssid, pass);

            int cont = 0;
            while (cont < 10 && WiFiMulti.run() != WL_CONNECTED)
            {
                cont++;

                Serial.print("WIFI -> trying connection: ");
                Serial.print(ssid);
                Serial.print(" - ");
                Serial.println(pass);
                delay(200);
            }

            if (WiFiMulti.run() == WL_CONNECTED)
            {
                this->_rgbled.blink_green();
                this->on = true;
                Serial.println("WIFI -> START");
            }
            else
            {
                this->_rgbled.blink_red();
                Serial.println("WIFI -> ERROR");
            }
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
        if (this->on && WiFiMulti.run() != WL_CONNECTED)
        {
            this->stop();
        }
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

    std::vector<String> get_wifi_list()
    { 
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        delay(100);

        std::vector<String> list;

        Serial.println("WIFI -> scan start");
 
        int n = WiFi.scanNetworks();
        Serial.println("WIFI -> scan done");
        if (n == 0)
        {
            Serial.println("WIFI -> no networks found");
        }
        else
        { 
            for (int i = 0; i < n; ++i)
            {
                // Print SSID and RSSI for each network found
                // Serial.print(i + 1);
                // Serial.print(": ");
                // Serial.print(WiFi.SSID(i));
                // Serial.print(" (");
                // Serial.print(WiFi.RSSI(i));
                // Serial.print(")");
                // Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");

                // std::string data = F(WiFi.SSID(i)) + " (" + F(WiFi.RSSI(i)) + ")" + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*"));
                String data = WiFi.SSID(i);
                list.push_back(data);
            }
        }
        // Serial.println("");

        return list;
    }
};
