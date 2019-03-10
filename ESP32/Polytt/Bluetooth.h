#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Regexp.h>
//https://github.com/nickgammon/Regexp

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "91e88e4d-66b6-40b7-aa14-d5af542a7f0b"
#define CHARACTERISTIC_UUID "19a09ba4-51f4-45eb-a2d9-bec08dad539e"



Storage _ble_storage = Storage(false);
bool _ble_device_connected = false;
bool _ble_old_device_connected = false;
BLECharacteristic *_ble_p_characteristic = NULL;
BLEServer *_ble_p_server = NULL;
String msg = "";

class ClientCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *_ble_p_characteristic)
    {
      std::string value = _ble_p_characteristic->getValue();

      if (value.length() > 0)
      {

        String chunk = "";
        for (int i = 0; i < value.length(); i++) {
          chunk +=  value[i];
        }


        MatchState ms;
        
        char copy[20];
        chunk.toCharArray(copy, 20); 
        ms.Target(copy);
        unsigned int count = ms.MatchCount ("^<MSG>");
 
        if (count > 0) {
          Serial.println("Msg init !");
          msg = "";
        }

        msg += chunk;

        
        MatchState ms2;
        char copy2[msg.length()];
        msg.toCharArray(copy2, msg.length()); 
        ms2.Target(copy2);
        unsigned int count2 = ms2.MatchCount ("MSG> *$");
  
          
        if (count2 > 0) {
          Serial.println("Msg end!");
          Serial.println(msg);
          msg = "";
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
    std::string device_name;
    BLEService *ble_service = NULL;
    bool on = false;



  public:
    Bluetooth(std::string device_name, Storage &storage)
    {
      this->device_name = device_name;
      _ble_storage = storage;
    }

    void start()
    {
      // Create the BLE Device
      BLEDevice::init(this->device_name);

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

      // Start the service
      this->ble_service->start();

      // Start advertising
      BLEAdvertising *p_advertising = BLEDevice::getAdvertising();
      p_advertising->addServiceUUID(SERVICE_UUID);
      p_advertising->setScanResponse(false);
      p_advertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
      BLEDevice::startAdvertising();
      Serial.println("Waiting a client connection to notify...");

      this->on = true;
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
      // Stop the service
      this->ble_service->stop();
      this->on = false;
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
          Serial.println("start advertising");
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
