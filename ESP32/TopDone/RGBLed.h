#include "Arduino.h"
#include <analogWrite.h>
//https://github.com/ERROPiX/ESP32_AnalogWrite/blob/master/library.json
#define PIN_LED_RED 23
#define PIN_LED_GREEN 18
#define PIN_LED_BLUE 19

class RGBLed
{

  private:   
  
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
        analogWrite(PIN_LED_RED, _red);
        analogWrite(PIN_LED_GREEN, _green);
        analogWrite(PIN_LED_BLUE, _blue);

        _red = 0;
        _green = 0;
        _blue = 0; 
    }

  public:
    RGBLed()
    {
    }

    void start()
    {
        analogWriteResolution(PIN_LED_RED, 12);
        analogWriteResolution(PIN_LED_GREEN, 12);
        analogWriteResolution(PIN_LED_BLUE, 12);
    }

    void loop_test()
    {
        color(_cont, _cont, _cont);

        _cont += _increase ? 5 : -5;

        if (_increase && _cont > 255)
        {
            _increase = false;
        }

        if (!_increase && _cont <= 0)
        {
            _increase = true;
        }

        delay(10);
    }

    void loop()
    {
        show();
    }

    void color(int R, int G, int B)
    {
        _red = ((R * (255 - _red_c)) / 255);
        _green = ((G * (255 - _green_c)) / 255);
        _blue = ((B * (255 - _blue_c)) / 255);
    }

    void red() { color(255, 0, 0); }
    void green() { color(0, 255, 0); }
    void blue() { color(0, 0, 255); }
    void cyan() { color(0, 255, 255); }
    void yellow() { color(255, 255, 0); }
    void magenta() { color(255, 0, 255); }
    void white() { color(255, 255, 255); }
    void black() { color(0, 0, 0); }

    void blink_red(int _blinks = 10, int _delay = 200)
    {
        blink(255, 0, 0, _blinks, _delay);
    }
    void blink_green(int _blinks = 10, int _delay = 200)
    {
        blink(0, 255, 0, _blinks, _delay);
    }
    void blink_blue(int _blinks = 10, int _delay = 200)
    {
        blink(0, 0, 255, _blinks, _delay);
    }
    void blink_cyan(int _blinks = 10, int _delay = 200)
    {
        blink(0, 255, 255, _blinks, _delay);
    }
    void blink_yellow(int _blinks = 10, int _delay = 200)
    {
        blink(255, 255, 0, _blinks, _delay);
    }
    void blink_magenta(int _blinks = 10, int _delay = 200)
    {
        blink(255, 0, 255, _blinks, _delay);
    }
    void blink_white(int _blinks = 10, int _delay = 200)
    {
        blink(255, 255, 255, _blinks, _delay);
    }
    void blink_black(int _blinks = 10, int _delay = 200)
    {
        blink(0, 0, 0, _blinks, _delay);
    }

    void blink(int R, int G, int B, int _blinks = 10, int _delay = 200)
    {
        int i;
        for (i = 0; i < _blinks; i = i + 1)
        {
            color( (i % 2 != 0) ? R : 0, (i % 2 != 0) ? G : 0, (i % 2 != 0) ? B : 0);
            show();
            delay(_delay);
        }
        black();
    }
};

RGBLed rgbLed = RGBLed();