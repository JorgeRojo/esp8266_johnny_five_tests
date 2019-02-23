#include <EEPROM.h>   
 

typedef struct
{ 
    int calibration_saved;
    int ax_offset;
    int ay_offset;
    int az_offset;
    int gx_offset;
    int gy_offset;
    int gz_offset;

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
            size_t size = sizeof(this->data);
            EEPROM.begin(size * 2);
            EEPROM_writeAnything(0, this->data);
            EEPROM.commit();
        } 

        void _loadStorage()
        {
            size_t size = sizeof(this->data);
            EEPROM.begin(size * 2);
            EEPROM_readAnything(0, this->data);

            // reset storage
            if(this->_reset) { 
                this->data.calibration_saved = 0;
            }
        }

        void _print() { 
            Serial.print("\t"); Serial.print(this->data.calibration_saved); 
            Serial.print("\t"); Serial.print(this->data.ax_offset); 
            Serial.print("\t"); Serial.print(this->data.ay_offset); 
            Serial.print("\t"); Serial.print(this->data.az_offset); 
            Serial.print("\t"); Serial.print(this->data.gx_offset); 
            Serial.print("\t"); Serial.print(this->data.gy_offset); 
            Serial.print("\t"); Serial.print(this->data.gz_offset);    
            Serial.println(""); 
        }

	public:
        Storage_t data;
        
		Storage(bool reset) 
        { 
            this->_reset = reset;
		} 

        void print()
        { 
            this->_loadStorage();   
            Serial.println(""); 
            Serial.print("Saved: ");
            this->_print();
        }

        void save()
        { 
            this->data.calibration_saved = 1;
   
            Serial.println(""); 
            Serial.print("Calibration saving... \t");
            this->_print();

            this->_saveStorage();
            Serial.println("Calibration saved!");
        }

        void load (bool print = false) {
            this->_loadStorage();
            if(print) { 
                Serial.println(""); 
                Serial.print("Loaded: ");
                this->_print();
            } 
        }
 
 

};
 
