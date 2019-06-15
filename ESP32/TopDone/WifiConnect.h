#include <Arduino.h>
#include <WiFi.h>

class WifiConnect
{
private:
public:
    WifiConnect()
    {
    }

    std::vector<String> getWifiList()
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
   
                list.push_back(WiFi.SSID(i));
            }
        }
        // Serial.println("");

        return list;
    }
};