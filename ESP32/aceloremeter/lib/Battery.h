/**/

#ifndef Battery_h
#define Battery_h
#endif 
#include <Arduino.h>
#include "./Battery.cpp"

class Battery
{
  public:
    Battery(int pin);
    float get();

  private:
    int _pin;
    unsigned int _raw;
    float _maxVolt;
    float _currentVolt;
};