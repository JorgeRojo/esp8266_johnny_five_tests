#include <EEPROM.h>


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
  bool wifi_connection_saved;
  char* wifi_ssid;
  char* wifi_password;
  
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

class Storage {
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
      if (_reset) {
        data.calibration_saved = false;
      }
    }

    void _print() {
      Serial.print("\t"); Serial.print(data.calibration_saved);
      Serial.print("\t"); Serial.print(data.ax_offset);
      Serial.print("\t"); Serial.print(data.ay_offset);
      Serial.print("\t"); Serial.print(data.az_offset);
      Serial.print("\t"); Serial.print(data.gx_offset);
      Serial.print("\t"); Serial.print(data.gy_offset);
      Serial.print("\t"); Serial.print(data.gz_offset);
      Serial.println("");
    }

  public:
    Storage_t data;

    Storage(bool reset)
    {
      _reset = reset;
    }

    void print()
    {
      _loadStorage();
      Serial.println("");
      Serial.print("Saved: ");
      _print();
    }

    void save()
    {
      data.calibration_saved = true;

      Serial.println("");
      Serial.print("Calibration saving... \t");
      _print();

      _saveStorage();
      Serial.println("Calibration saved!");
    }

    void load (bool print = false) {
      _loadStorage();
      if (print) {
        Serial.println("");
        Serial.print("Loaded: ");
        _print();
      }
    }
    
};
