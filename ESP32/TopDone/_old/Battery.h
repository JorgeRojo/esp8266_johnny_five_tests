#include "Arduino.h"
#include <analogWrite.h>
#define RAW_MAX 1350.0 // initial max row
#define RAW_MIN 1120.0 // initial min row
#define RAW_EM 5       // row margin error

class Battery
{

  private:
    byte _pin; 
    float _max = RAW_MAX; 
    float _last = RAW_MIN;
    float _level = 0;
    float _last_level = 0;

  public:
    Battery(byte pin)
    {
        _pin = pin;
    }

    void start()
    {
        analogWriteResolution(_pin, 12);
    }

    float loop()
    {
        unsigned int _raw = analogRead(_pin);

    

        if (_last < _raw && (_raw - _last) > RAW_EM)
        {
            _last += RAW_EM;
        }
        else if (_last > _raw && (_last - _raw) > RAW_EM)
        {
            _last -= RAW_EM;
        }
        else
        {
            _last = _raw;
        }

        if (_last > _max) 
        {
            _max = _last;
        }

        _level = (100 * (_last - RAW_MIN)) / (_max - RAW_MIN);
        
        Serial.println(_last);
//        Serial.println(_level);

        return _level;
    }

    void on_level_change(void (*notify)(float))
    { 
        float dif = _last_level - _level; 
        if (dif >= 10 || dif <= -10)
        {
            _last_level = _level;
            notify(_level);
            Serial.print("BATTERY -> RAW\t");
            Serial.println(_last);
        }
    }
};
