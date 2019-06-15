#include <Arduino.h>
#include <WiFi.h>

class WifiConnect
{
private:
public:
    WifiConnect()
    {
    }

    static std::vector<String> getWifiList()
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
                list.push_back(WiFi.SSID(i));
            }
        }

        return list;
    }

    static bool checkConnection(const char *ssid, const char *pass, bool save= false)
    {
        WiFi.begin(ssid, pass);

        int attempts = 0;
        bool isConnected = false;

        do
        {
            attempts++;
            isConnected = WiFi.status() != WL_CONNECTED;
            delay(200);
        } while (attempts <= 10 && !isConnected);

        if(isConnected && save) {
            Storage storage = Storage();
            storage.data.wifi_ssid = const_cast<char*>(ssid); // ssid;
            storage.data.wifi_pass = const_cast<char*>(pass);
            storage.save(true);
        }

        return isConnected;
    }
};