
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID "91e88e4d-66b6-40b7-aa14-d5af542a7f0b"
#define CHARACTERISTIC_UUID "19a09ba4-51f4-45eb-a2d9-bec08dad539e"

static std::string toStdString(String string)
{
    std::string res = "";
    for (int i = 0; i < string.length(); i++)
    {
        res += string[i];
    }
    return res;
}

bool BleConnected = false;
bool BleAdStarted = false;

class ClientCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string value = pCharacteristic->getValue();

        Serial.println(value.c_str());
    };
};

class ServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        BleConnected = true;
    };

    void onDisconnect(BLEServer *pServer)
    {
        BleConnected = false;
    }
};

class BLEConnect
{
private:
    std::string deviceName;
    BLECharacteristic *pCharacteristic = NULL;
    BLEServer *pServer = NULL;
    BLEService *pService = NULL;
    std::vector<String> (*getWifiList)();

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

    void sendLongMessage(std::string msg)
    {
        pCharacteristic->setValue("<BLEMSG>");
        pCharacteristic->notify();

        int chunkLength = 20;
        for (int i = 0; i < msg.length(); i += chunkLength)
        {
            std::string chunk = stringChunk(msg, i, chunkLength);
            pCharacteristic->setValue(chunk);
            pCharacteristic->notify();
        }

        pCharacteristic->setValue("</BLEMSG>");
        pCharacteristic->notify();
    }

public:
    BLEConnect(std::vector<String> (*_getWifiList)())
    {
        deviceName = getDeviceID();
        getWifiList = _getWifiList;
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

        if (!BleConnected && !BleAdStarted)
        {
            pServer->startAdvertising();
            Serial.println("BLE -> start advertising");
            BleAdStarted = true;
        }

        if (BleConnected && BleAdStarted)
        {
            BleAdStarted = false;

            std::vector<String> wifiList = getWifiList();
  
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
            
            sendLongMessage( toStdString(msg) );
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