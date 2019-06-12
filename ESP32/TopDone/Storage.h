#include <Arduino.h>
#include <EEPROM.h>

class Storage
{
private:
    typedef struct
    {
        // orientation
        bool mpu6050_saved;
        float mpu6050_x_offset;
        float mpu6050_y_offset;
        float mpu6050_z_offset;

        // wifi
        char *wifi_ssid;
        char *wifi_password;

    } Storage_t;

    bool reset;

    int writeAnything(int ee, const Storage_t &data)
    {
        const byte *p = (const byte *)(const void *)&data;
        unsigned int i;
        for (i = 0; i < sizeof(data); i++)
            EEPROM.write(ee++, *p++);
        return i;
    }

    int readAnything(int ee, Storage_t &data)
    {
        byte *p = (byte *)(void *)&data;
        unsigned int i;
        for (i = 0; i < sizeof(data); i++)
            *p++ = EEPROM.read(ee++);
        return i;
    }

    void saveStorage()
    {
        size_t size = sizeof(data);
        EEPROM.begin(size * 2);
        writeAnything(0, data);
        EEPROM.commit();
    }

    void loadStorage()
    {
        size_t size = sizeof(data);
        EEPROM.begin(size * 2);
        readAnything(0, data);

        // reset storage
        if (reset)
        {

            Serial.println("Storage -> reset");
            data.mpu6050_saved = false;
            data.wifi_ssid = "";
            data.wifi_password = "";
        }
    }

    void _print()
    {
        Serial.println("STRG -> *** data list ***");

        Serial.println("  --- calibration ---  ");
        Serial.print("    mpu6050_saved:\t");
        Serial.println(data.mpu6050_saved);
        Serial.print("    mpu6050_x_offset:\t");
        Serial.println(data.mpu6050_x_offset);
        Serial.print("    mpu6050_y_offset:\t");
        Serial.println(data.mpu6050_y_offset);
        Serial.print("    mpu6050_z_offset:\t");
        Serial.println(data.mpu6050_z_offset);

        // Serial.println("  --- wifi ---  ");
        // Serial.print("    wifi_ssid:\t");
        // Serial.println(data.wifi_ssid);
        // Serial.print("    wifi_password:\t");
        // Serial.println(data.wifi_password);
    }

public:
    Storage_t data;

    Storage(bool doReset)
    {
        reset = doReset;
    }

    void print()
    {
        loadStorage();
        _print();
    }

    void save(bool doPrint = false)
    {
        Serial.println("STRG -> saving... \t");
        saveStorage();
        if (doPrint)
        {
            _print();
        }
    }

    void load(bool doPrint = false)
    {
        loadStorage();
        if (doPrint)
        {
            Serial.println("");
            Serial.print("STRG -> Loaded: ");
            _print();
        }
    }
};
