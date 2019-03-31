#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Regexp.h>
#include <ArduinoJson.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "91e88e4d-66b6-40b7-aa14-d5af542a7f0b"
#define CHARACTERISTIC_UUID "19a09ba4-51f4-45eb-a2d9-bec08dad539e"

Storage _ble_storage = Storage(false);
bool _ble_device_connected = false;
bool _ble_old_device_connected = false;
BLECharacteristic *_ble_p_characteristic = NULL;
BLEServer *_ble_p_server = NULL;
String _ble_tmp_msg = "";

void parse_msg(char *msg)
{
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, msg);

    if (error)
    {
        Serial.print(F("BLE -> deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }

    const char *wifi_ssid = doc["wifi_ssid"];
    const char *wifi_pass = doc["wifi_pass"];

    Serial.print("BLE -> wifi ssid: ");
    Serial.println(wifi_ssid);
    Serial.print("BLE -> wifi pass: ");
    Serial.println(wifi_pass);
}

class ClientCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *_ble_p_characteristic)
    {
        std::string value = _ble_p_characteristic->getValue();

        if (value.length() > 0)
        {

            String chunk = "";
            for (int i = 0; i < value.length(); i++)
            {
                chunk += value[i];
            }

            //find msg parts
            MatchState ms;
            unsigned int count = 0;

            char c_chunk[chunk.length()];
            chunk.toCharArray(c_chunk, chunk.length());
            ms.Target(c_chunk);
            count = ms.MatchCount("^<MSG>");

            if (count > 0)
            {
                _ble_tmp_msg = "";
            }

            _ble_tmp_msg += chunk;

            char c_msg[(_ble_tmp_msg + " ").length()];
            (_ble_tmp_msg + " ").toCharArray(c_msg, (_ble_tmp_msg + " ").length());
            ms.Target(c_msg);
            count = ms.MatchCount("<#MSG> *");

            if (count > 0)
            {

                ms.GlobalReplace("[^<]*<MSG> *", "");
                ms.GlobalReplace("<#MSG>.*", "");

                parse_msg(c_msg);

                _ble_tmp_msg = "";
            }
        }
    };
};

class ServerCallbacks : public BLEServerCallbacks
{

    void onConnect(BLEServer *_ble_p_server)
    {
        _ble_device_connected = true;
        BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer *_ble_p_server)
    {
        _ble_device_connected = false;
    };
};

class Bluetooth
{

  private: 
    BLEService *ble_service = NULL;
    bool on = false;

  public:
    Bluetooth(Storage &storage)
    {
        _ble_storage = storage;
    } 
      
    void setup(std::string device_name)
    { 

        _ble_storage.load(false);

        // Create the BLE Device
        BLEDevice::init( device_name );

        // Create the BLE Server
        _ble_p_server = BLEDevice::createServer();
        _ble_p_server->setCallbacks(new ServerCallbacks());

        // Create the BLE Service
        this->ble_service = _ble_p_server->createService(SERVICE_UUID);

        // Create a BLE Characteristic
        _ble_p_characteristic = this->ble_service->createCharacteristic(
            CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ |
                BLECharacteristic::PROPERTY_WRITE |
                BLECharacteristic::PROPERTY_NOTIFY |
                BLECharacteristic::PROPERTY_INDICATE);

        // Create BLE Client Service
        _ble_p_characteristic->setCallbacks(new ClientCallbacks());

        // Create a BLE Descriptor
        _ble_p_characteristic->addDescriptor(new BLE2902());

        this->on = false;
    }

    void start()
    {
        if (!this->on)
        {
            // Start the service
            this->ble_service->start();

            // Start advertising
            BLEAdvertising *p_advertising = BLEDevice::getAdvertising();
            p_advertising->addServiceUUID(SERVICE_UUID);
            p_advertising->setScanResponse(false);
            p_advertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
            BLEDevice::startAdvertising();
            Serial.println("BLE -> Waiting a client connection to notify...");

            this->on = true;
            Serial.println("BLE -> START");
        }
    }

    void sendToDevice(std::string message)
    {
        if (_ble_device_connected)
        {
            _ble_p_characteristic->setValue(message);
            _ble_p_characteristic->notify();
        }
    }

    void stop()
    {
        if (this->on)
        {
            this->ble_service->stop();
            this->on = false;
            Serial.println("BLE -> STOP");
        }
    }

    void loop()
    {
        if (this->on)
        {
            // disconnecting
            if (!_ble_device_connected && _ble_old_device_connected)
            {
                delay(500);                        // give the bluetooth stack the chance to get things ready
                _ble_p_server->startAdvertising(); // restart advertising
                Serial.println("BLE -> start advertising");
                _ble_old_device_connected = _ble_device_connected;
            }

            // connecting
            if (_ble_device_connected && !_ble_old_device_connected)
            {
                _ble_old_device_connected = _ble_device_connected;
            }
        }
    }
};
