//https://github.com/espressif/arduino-esp32/tree/master/libraries/EEPROM/examples
#include <ArduinoJson.h>
#include <EEPROM.h>

typedef struct
{
    bool mpuSaved;
    long mpuX;
    long mpuY;
    long mpuZ;
    char *wifiName;
    char *wifiPass;
    char *wifiHost;
    int wifiPort;
} Storage_t;

class Storage
{
private:
    String strData;
    Storage_t data;
    DeserializationError error;
    StaticJsonDocument<1000> doc;
    bool reset;

    void parseDocData()
    {
        const bool mpuSaved = doc["mpuSaved"];
        data.mpuSaved = mpuSaved;

        const long mpuX = doc["mpuX"];
        data.mpuX = mpuX;

        const long mpuY = doc["mpuY"];
        data.mpuY = mpuY;

        const long mpuZ = doc["mpuZ"];
        data.mpuZ = mpuZ;

        const char *wifiName = doc["wifiName"];
        data.wifiName = const_cast<char *>(wifiName);

        const char *wifiPass = doc["wifiPass"];
        data.wifiPass = const_cast<char *>(wifiPass);

        const char *wifiHost = doc["wifiHost"];
        data.wifiHost = const_cast<char *>(wifiHost);

        const int wifiPort = doc["wifiPort"];
        data.wifiPort = wifiPort;
    }

    void getSavedStorage()
    {
        strData = EEPROM.readString(0);
        error = deserializeJson(doc, strData);

        if (!error)
        {
            parseDocData();
        }
        else
        {
            Serial.println("STORAGE -> Json deserialize error...");
        }
    }

    void setSavedDoc()
    {
        getSavedStorage();
        doc["mpuSaved"] = data.mpuSaved;
        doc["mpuX"] = data.mpuX;
        doc["mpuY"] = data.mpuY;
        doc["mpuZ"] = data.mpuZ;
        doc["wifiName"] = data.wifiName;
        doc["wifiPass"] = data.wifiPass;
        doc["wifiHost"] = data.wifiHost;
        doc["wifiPort"] = data.wifiPort;
    }

    void saveStorage()
    {
        strData = "";
        serializeJson(doc, strData);
        parseDocData();
        EEPROM.writeString(0, strData);
        EEPROM.commit();
    }

public:
    Storage()
    {
    }

    void setup(bool doPrint = false)
    {
        if (!EEPROM.begin(1000))
        {
            delay(1000);
            ESP.restart();
        }

        getSavedStorage();
        if (doPrint)
        {
            Serial.printf("\nSTORAGE -> Loading...\n");
            print();
        }
    }

    void print()
    {
        getSavedStorage();

        Serial.printf(">>>----->>> mpuSaved %d \n", data.mpuSaved);
        Serial.printf(">>>----->>> mpuX %d \n", data.mpuX);
        Serial.printf(">>>----->>> mpuY %d \n", data.mpuY);
        Serial.printf(">>>----->>> mpuZ %d \n", data.mpuZ);
        Serial.printf(">>>----->>> wifiName %s \n", data.wifiName);
        Serial.printf(">>>----->>> wifiPass %s \n", data.wifiPass);
        Serial.printf(">>>----->>> wifiHost %s \n", data.wifiHost);
        Serial.printf(">>>----->>> wifiPort %d \n", data.wifiPort);
    }

    void saveMpu(const int mpuX, const int mpuY, const int mpuZ)
    {
        setSavedDoc();
        doc["mpuSaved"] = true;
        doc["mpuX"] = mpuX;
        doc["mpuY"] = mpuY;
        doc["mpuZ"] = mpuZ;

        Serial.println("STORAGE -> Saving mpu...");
        saveStorage();
    }

    void saveWifi(const char *wifiName, const char *wifiPass, const char *wifiHost, const int wifiPort)
    {
        setSavedDoc();
        doc["wifiName"] = wifiName;
        doc["wifiPass"] = wifiPass;
        doc["wifiHost"] = wifiHost;
        doc["wifiPort"] = wifiPort;

        Serial.println("STORAGE -> Saving wifi...");
        saveStorage();
    }

    Storage_t getData()
    {
        return data;
    }
};

Storage storage = Storage();