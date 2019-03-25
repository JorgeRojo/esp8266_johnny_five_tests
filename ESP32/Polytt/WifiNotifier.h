#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

class WifiNotifier
{
  private: 
    Storage _storage = Storage(false);
    
    void _connect() 
    {   
    }
    
  public:
    WifiNotifier(Storage &storage)
    { 
      this->_storage = storage;  
    }

    void start()
    {
        this->_storage.load(false);
    }

    void loop()
    {
      
    }

    bool testConnection () 
    {
        if (!this->_storage.data.wifi_ssid || !this->_storage.data.wifi_password) {
            return false; 
        }
    }

};
