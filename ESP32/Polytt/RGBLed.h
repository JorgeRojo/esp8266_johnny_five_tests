#include "Arduino.h"
#include <analogWrite.h>
//https://github.com/ERROPiX/ESP32_AnalogWrite/blob/master/library.json

class RGBLed
{

  private:
    byte pin_red;
    byte pin_green;
    byte pin_blue;

    int _red_c = 60;
    int _green_c = 100;
    int _blue_c = 0;

    int _cont;
    bool _increase = true;

    bool _blick_on = true;

    void blink()
    {
    }

  public:
    RGBLed(byte _pin_red, byte _pin_green, byte _pin_blue)
    {
        this->pin_red = _pin_red;
        this->pin_green = _pin_green;
        this->pin_blue = _pin_blue;
    }

    void start()
    {
        analogWriteResolution(this->pin_red, 12);
        analogWriteResolution(this->pin_green, 12);
        analogWriteResolution(this->pin_blue, 12);
    }

    void loop_test()
    {
        this->color(this->_cont, this->_cont, this->_cont);

        this->_cont += this->_increase ? 5 : -5;

        if (this->_increase && this->_cont > 255)
        {
            this->_increase = false;
        }

        if (!this->_increase && this->_cont <= 0)
        {
            this->_increase = true;
        }

        delay(10);
    }

    void loop()
    {
    }

    void color(int R, int G, int B)
    {
        analogWrite(this->pin_red, ((R * (255 - this->_red_c)) / 255));
        analogWrite(this->pin_green, ((G * (255 - this->_green_c)) / 255));
        analogWrite(this->pin_blue, ((B * (255 - this->_blue_c)) / 255));
    }

    void white() 
    {
       this->color(255, 255, 255);      
    }

    void black() 
    {
       this->color(0, 0, 0);      
    }

    void blink(int R, int G, int B, int _delay)
    {

        this->color(
            this->_blick_on ? R : 0,
            this->_blick_on ? G : 0,
            this->_blick_on ? B : 0);
        this->_blick_on = !this->_blick_on;
        delay(_delay);
    }
};
