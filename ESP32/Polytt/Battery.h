#include "Arduino.h"
#include <analogWrite.h>
#define ROW_MAX 2100.0 // initial max row
#define RAW_EM 5       // row margin error

class Battery
{

  private:
    byte pin;
    unsigned int _raw;
    float _max = ROW_MAX;
    float _last = ROW_MAX;
    Storage _storage = Storage(false);

    void _store_max_raw()
    {
        if (this->_max < this->_last)
        {
            this->_max = this->_last;
            this->_storage.data.max_raw_battery = this->_last;

            Serial.print("BATT -> load max battery ");
            Serial.println(this->_last);
            this->_storage.save(false);
        }
    }

    void _set_max_raw()
    {
        if (this->_storage.data.max_raw_battery > this->_max)
        {
            this->_max = this->_storage.data.max_raw_battery;
        }
        Serial.print("BATT -> Max initial battery raw: ");
        Serial.println(this->_max);
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
        this->_set_max_raw();
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

        this->_store_max_raw();

        float level = (100 * this->_last) / this->_max;

        return level;
    }
};
