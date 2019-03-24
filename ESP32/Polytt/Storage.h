#include <EEPROM.h>

typedef struct
{
    // battery
    float max_raw_battery;

    // orientation
    bool calibration_saved;
    int ax_offset;
    int ay_offset;
    int az_offset;
    int gx_offset;
    int gy_offset;
    int gz_offset;

    // wifi
    bool wifi_connection_saved;
    char *wifi_ssid;
    char *wifi_password;

} Storage_t;

int EEPROM_writeAnything(int ee, const Storage_t &data)
{
    const byte *p = (const byte *)(const void *)&data;
    unsigned int i;
    for (i = 0; i < sizeof(data); i++)
        EEPROM.write(ee++, *p++);
    return i;
}

int EEPROM_readAnything(int ee, Storage_t &data)
{
    byte *p = (byte *)(void *)&data;
    unsigned int i;
    for (i = 0; i < sizeof(data); i++)
        *p++ = EEPROM.read(ee++);
    return i;
}

class Storage
{
  private:
    byte _pin;
    bool _reset;

    void _saveStorage()
    {
        size_t size = sizeof(data);
        EEPROM.begin(size * 2);
        EEPROM_writeAnything(0, data);
        EEPROM.commit();
    }

    void _loadStorage()
    {
        size_t size = sizeof(data);
        EEPROM.begin(size * 2);
        EEPROM_readAnything(0, data);

        // reset storage
        if (_reset)
        {
            data.calibration_saved = false;
        }
    }

    void _print()
    { 
        Serial.println("STRG -> *** data list ***");

        Serial.println("  --- battery ---");
        Serial.print("    max_raw_battery:\t"); Serial.println(data.max_raw_battery);

        Serial.println("  --- calibration ---  ");
        Serial.print("    calibration_saved:\t"); Serial.println(data.calibration_saved);
        Serial.print("    ax_offset:\t"); Serial.println(data.ax_offset);
        Serial.print("    ay_offset:\t"); Serial.println(data.ay_offset);
        Serial.print("    az_offset:\t"); Serial.println(data.az_offset);
        Serial.print("    gx_offset:\t"); Serial.println(data.gx_offset);
        Serial.print("    gy_offset:\t"); Serial.println(data.gy_offset);
        Serial.print("    gz_offset:\t"); Serial.println(data.gz_offset);
        
        Serial.println("  --- wifi ---  ");
        Serial.print("    wifi_connection_saved:\t"); Serial.println(data.wifi_connection_saved);
        Serial.print("    wifi_ssid:\t"); Serial.println(data.wifi_ssid);
        Serial.print("    wifi_password:\t"); Serial.println(data.wifi_password);

    }

  public:
    Storage_t data;

    Storage(bool reset)
    {
        _reset = reset;
    }

    void print()
    {
        this->_loadStorage(); 
        this->_print();
    }

    void save()
    { 
        Serial.println("STRG -> saving... \t"); 
        this->_saveStorage();
        this->_print();
    }

    void load(bool print = false)
    {
        this->_loadStorage();
        if (print)
        {
            Serial.println("");
            Serial.print("STRG -> Loaded: ");
            this->_print();
        }
    }
};
