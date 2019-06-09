#include "Arduino.h"
#include <analogWrite.h>
//https://github.com/ERROPiX/ESP32_AnalogWrite/blob/master/library.json

class RGBLed
{

  private:
    byte pin_red;
    byte pin_green;
    byte pin_blue;

    int _red_c = 0;
    int _green_c = 100;
    int _blue_c = 0;

    int _red = 0;
    int _green = 0;
    int _blue = 0;

    int _cont;
    bool _increase = true;

    void show() 
    {
        analogWrite(this->pin_red, this->_red);
        analogWrite(this->pin_green, this->_green);
        analogWrite(this->pin_blue, this->_blue);

        this->_red = 0;
        this->_green = 0;
        this->_blue = 0; 
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
        this->show();
    }

    void color(int R, int G, int B)
    {
        this->_red = ((R * (255 - this->_red_c)) / 255);
        this->_green = ((G * (255 - this->_green_c)) / 255);
        this->_blue = ((B * (255 - this->_blue_c)) / 255);
    }

    void red() { this->color(255, 0, 0); }
    void green() { this->color(0, 255, 0); }
    void blue() { this->color(0, 0, 255); }
    void cyan() { this->color(0, 255, 255); }
    void yellow() { this->color(255, 255, 0); }
    void magenta() { this->color(255, 0, 255); }
    void white() { this->color(255, 255, 255); }
    void black() { this->color(0, 0, 0); }

    void blink_red(int _blinks = 10, int _delay = 200)
    {
        this->blink(255, 0, 0, _blinks, _delay);
    }
    void blink_green(int _blinks = 10, int _delay = 200)
    {
        this->blink(0, 255, 0, _blinks, _delay);
    }
    void blink_blue(int _blinks = 10, int _delay = 200)
    {
        this->blink(0, 0, 255, _blinks, _delay);
    }
    void blink_cyan(int _blinks = 10, int _delay = 200)
    {
        this->blink(0, 255, 255, _blinks, _delay);
    }
    void blink_yellow(int _blinks = 10, int _delay = 200)
    {
        this->blink(255, 255, 0, _blinks, _delay);
    }
    void blink_magenta(int _blinks = 10, int _delay = 200)
    {
        this->blink(255, 0, 255, _blinks, _delay);
    }
    void blink_white(int _blinks = 10, int _delay = 200)
    {
        this->blink(255, 255, 255, _blinks, _delay);
    }
    void blink_black(int _blinks = 10, int _delay = 200)
    {
        this->blink(0, 0, 0, _blinks, _delay);
    }

    void blink(int R, int G, int B, int _blinks = 10, int _delay = 200)
    {
        int i;
        for (i = 0; i < _blinks; i = i + 1)
        {
            this->color( (i % 2 != 0) ? R : 0, (i % 2 != 0) ? G : 0, (i % 2 != 0) ? B : 0);
            this->show();
            delay(_delay);
        }
        this->black();
    }
};
