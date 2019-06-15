
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <regex>
#include <ArduinoJson.h>

#define SERVICE_UUID "91e88e4d-66b6-40b7-aa14-d5af542a7f0b"
#define CHARACTERISTIC_UUID "19a09ba4-51f4-45eb-a2d9-bec08dad539e"
#define BLE_CHUNK_SIZE 20

bool BLEconnected = false;

static std::string toStdString(String string)
{
    std::string res = "";
    for (int i = 0; i < string.length(); i++)
    {
        res += string[i];
    }
    return res;
}

std::string stringChunk(std::string msg, int start, int end)
{
    std::string chunk = "";
    for (int i = start; i < (start + end); i++)
    {
        if (msg.length() > i)
        {
            chunk += msg[i];
        }
    }
    return chunk;
}

void sendLongMessage(BLECharacteristic *pCharacteristic, std::string msg)
{
    pCharacteristic->setValue("<BLEMSG>");
    pCharacteristic->notify();

    for (int i = 0; i < msg.length(); i += BLE_CHUNK_SIZE)
    {
        std::string chunk = stringChunk(msg, i, BLE_CHUNK_SIZE);
        pCharacteristic->setValue(chunk);
        pCharacteristic->notify();
    }

    pCharacteristic->setValue("</BLEMSG>");
    pCharacteristic->notify();
}

bool BLECheckWifiConnection(std::string msg )
{
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, msg);

    if (error)
    {
        Serial.print(F("BLE -> deserializeJson() failed: "));
        Serial.println(error.c_str());
        return false;
    }

    const char *ssid = doc["wifi_ssid"];
    const char *pass = doc["wifi_pass"];
 
    return ssid && pass && WifiConnect::checkConnection(ssid, pass, true);
}

class ClientCallbacks : public BLECharacteristicCallbacks
{
    bool msgInit = false;
    std::string msg = "";

    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string value = pCharacteristic->getValue();

        if (std::regex_match(value, std::regex("<BLEMSG> *")))
        {
            msgInit = true;
            msg = "";
        }
        else if (std::regex_match(value, std::regex("</BLEMSG> *")))
        {
            bool connection = BLECheckWifiConnection(msg);
            sendLongMessage(pCharacteristic, connection ? "{\"wifi\":true}" : "{\"wifi\":false}");
            msgInit = false;
        }
        else if (msgInit)
        {
            msg += value;
        }
    };
};

class ServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        BLEconnected = true;
    };

    void onDisconnect(BLEServer *pServer)
    {
        BLEconnected = false;
    }
};

class BLEConnect
{
private:
    bool adStarted = false;
    std::string deviceName;
    BLECharacteristic *pCharacteristic = NULL;
    BLEServer *pServer = NULL;
    BLEService *pService = NULL;

public:
    BLEConnect()
    {
        deviceName = getDeviceID();
    }

    void start()
    {
        BLEDevice::init(deviceName);

        pServer = BLEDevice::createServer();
        pServer->setCallbacks(new ServerCallbacks());

        pService = pServer->createService(SERVICE_UUID);

        pCharacteristic = pService->createCharacteristic(
            CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ |
                BLECharacteristic::PROPERTY_WRITE |
                BLECharacteristic::PROPERTY_NOTIFY |
                BLECharacteristic::PROPERTY_INDICATE);

        pCharacteristic->setCallbacks(new ClientCallbacks());
        pCharacteristic->addDescriptor(new BLE2902());

        // start
        pService->start();

        //start advertising
        BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(SERVICE_UUID);
        pAdvertising->setScanResponse(false);
        pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
    }

    void loop()
    {
        delay(1000);

        if (!BLEconnected && !adStarted)
        {
            pServer->startAdvertising();
            Serial.println("BLE -> start advertising");
            adStarted = true;
        }

        if (BLEconnected && adStarted)
        {
            adStarted = false;

            std::vector<String> wifiList = WifiConnect::getWifiList();

            String msg = "[";
            for (int i = 0; i < wifiList.size(); i++)
            {
                msg.concat("\"");
                msg.concat(wifiList[i]);
                msg.concat("\"");
                if (i < (wifiList.size() - 1))
                {
                    msg.concat(",");
                }
            }
            msg.concat("]");

            sendLongMessage(pCharacteristic, toStdString(msg));
        }
    }

    std::string getDeviceID()
    {
        uint64_t chipid = ESP.getEfuseMac();
        uint16_t chip = (uint16_t)(chipid >> 32);
        char uid[23];
        snprintf(uid, 23, "TopDone-%04X%08X", chip, (uint32_t)chipid);
        return std::string(uid);
    }
};