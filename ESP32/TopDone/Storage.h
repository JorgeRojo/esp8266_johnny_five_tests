#include <Arduino.h>
#include <EEPROM.h>

class Storage
{
private:
    typedef struct
    {
        // orientation
        bool calibration_saved;
        int ax_offset;
        int ay_offset;
        int az_offset;
        int gx_offset;
        int gy_offset;
        int gz_offset;

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
            data.calibration_saved = false;
            data.wifi_ssid = "";
            data.wifi_password = "";
        }
    }

    void _print()
    {
        Serial.println("STRG -> *** data list ***");

        Serial.println("  --- calibration ---  ");
        Serial.print("    calibration_saved:\t");
        Serial.println(data.calibration_saved);
        Serial.print("    ax_offset:\t");
        Serial.println(data.ax_offset);
        Serial.print("    ay_offset:\t");
        Serial.println(data.ay_offset);
        Serial.print("    az_offset:\t");
        Serial.println(data.az_offset);
        Serial.print("    gx_offset:\t");
        Serial.println(data.gx_offset);
        Serial.print("    gy_offset:\t");
        Serial.println(data.gy_offset);
        Serial.print("    gz_offset:\t");
        Serial.println(data.gz_offset);

        Serial.println("  --- wifi ---  ");
        Serial.print("    wifi_ssid:\t");
        Serial.println(data.wifi_ssid);
        Serial.print("    wifi_password:\t");
        Serial.println(data.wifi_password);
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
