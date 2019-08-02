//https://github.com/Links2004/arduinoWebSockets/blob/master/examples/esp32/WebSocketClient/WebSocketClient.ino
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <SocketIoClient.h>

WiFiMulti WiFiMulti;
SocketIoClient webSocket;

bool socketConnected = false;

void on_connect(const char *payload, size_t length)
{
    socketConnected = true;
    Serial.print("WIFI -> socketConnected: ");
    Serial.println(socketConnected);
}

class WifiConnect
{
private:
    bool wifiConnected = false;

    void setWifiConnection(const char *ssid, const char *pass, bool save = false)
    {
        int attempts = 0;

        WiFiMulti.addAP(ssid, pass);

        while (attempts <= 100 && WiFiMulti.run() != WL_CONNECTED)
        {
            delay(10);
            attempts++;
        }

        // Serial.printf(">>>----->>> attempts %d \n", attempts);

        wifiConnected = attempts <= 100;

        if (wifiConnected && save)
        {
            Storage_t data = storage.getData();
            storage.saveWifi(ssid, pass, data.wifiHost, data.wifiPort);
        }
    }

public:
    WifiConnect()
    {
    }

    bool isWifiConnected()
    {
        return wifiConnected;
    }

    bool isSocketConnected()
    {
        return socketConnected;
    }

    bool validateWifiConnection(const char *ssid, const char *pass)
    {
        Serial.printf(">>>----->>> validateWifiConnection \n");
        setWifiConnection(ssid, pass, true);
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
                list.push_back(WiFi.SSID(i));
            }
        }

        return list;
    }

    std::string getDeviceID()
    {
        uint64_t chipid = ESP.getEfuseMac();
        uint16_t chip = (uint16_t)(chipid >> 32);
        char uid[23];
        snprintf(uid, 23, "TopDone-%04X%08X", chip, (uint32_t)chipid);
        return std::string(uid);
    }

    void sendFace(char *face)
    {

        std::string data = "{";
        data += "\"face\":";
        data += "\"";
        data += face;
        data += "\"";
        data += ",";
        data += "\"id\":";
        data += "\"";
        data += getDeviceID();
        data += "\"";
        data += "}";

        webSocket.emit("face", data.c_str());
    }

    void start()
    {
        Storage_t data = storage.getData();
        const char *ssid = data.wifiName;
        const char *pass = data.wifiPass;
        const char *host = data.wifiHost;
        int port = data.wifiPort;
        setWifiConnection(ssid, pass);

        Serial.print("WIFI -> wifiConnected: ");
        Serial.println(wifiConnected);

        if (wifiConnected)
        {
            webSocket.on("connect", on_connect);
            webSocket.begin(host, port, "/socket.io/??EIO=3&transport=websocket");
        }
    }

    void loop()
    {
        webSocket.loop();
    }
};

WifiConnect wifiConnect = WifiConnect();