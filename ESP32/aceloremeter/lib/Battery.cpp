/**/

#include <Arduino.h>
#include "./Barrier.h"
  
Battery::Battery(int pin, float maxVolt)
{
    pinMode(pin, OUTPUT);
    _pin = pin;
    _maxVolt = maxVolt;
}

float Battery::get()
{   
    _raw = analogRead(_pin); 
    float _res = 2;
    float _rawVolt = (4095 * (_rawVolt / 2)) / (3.3 * _res);

    _volt = (_maxVolt / 2) * (4095 / 3.3) (_raw / (4095 / 3.3))  ;
    return _volt;
}