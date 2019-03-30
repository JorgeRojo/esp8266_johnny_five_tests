#include "Arduino.h"
#include <analogWrite.h>
#define ROW_MAX 1550.0 // initial max row
#define ROW_MIN 1300.0 // initial max row
#define RAW_EM 1       // row margin error

class Battery
{

  private:
    byte pin;
    unsigned int _raw;
    float _max = ROW_MAX;
    float _min = ROW_MIN;
    float _last = ROW_MAX;
    Storage _storage = Storage(false);

    void store_raw()
    {
        if ( this->_last - this->_max > 20 )
        {
            this->_max = this->_last;
            this->_storage.data.max_raw_battery = this->_last; 
            this->_storage.save(false);
        }
        
        if ( this->_min - this->_last > 20 )
        {
            this->_min = this->_last;
            this->_storage.data.min_raw_battery = this->_last; 
            this->_storage.save(false); 
        }
    }

    void _set_raw()
    {
        if (this->_storage.data.max_raw_battery > this->_max)
        {
            this->_max = this->_storage.data.max_raw_battery;
        }
           
        if (this->_storage.data.min_raw_battery < this->_min)
        {
            this->_min = this->_storage.data.min_raw_battery;
        }
        Serial.print("BATT -> Max initial battery raw: ");
        Serial.println(this->_max);
        Serial.print("BATT -> Min initial battery raw: ");
        Serial.println(this->_min);
    }

  public:
    Battery(byte pin, Storage &storage)
    {
        this->pin = pin;
        analogWriteResolution(this->pin, 12);

        this->_storage = storage;
    }

    void start()
    {
        this->_storage.load(false);
        this->_set_raw();
    }

    float level()
    {
        _raw = analogRead(this->pin);

        if (this->_last < _raw && (_raw - this->_last) > RAW_EM)
        {
            this->_last += RAW_EM;
        }
        else if (this->_last > _raw && (this->_last - _raw) > RAW_EM)
        {
            this->_last -= RAW_EM;
        }
        else
        {
            this->_last = _raw;
        }

         
        Serial.print(this->_max);  
        Serial.print(" - ");
        Serial.print(this->_last); 
        Serial.print(" - ");
        Serial.println(this->_min); 

        this->store_raw();

        float level = (100 * this->_last) / this->_max;

        return level;
    }
};
