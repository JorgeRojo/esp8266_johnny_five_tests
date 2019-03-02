#define BATT_MAX_RAW 2320


class Battery {

  private:
    byte _pin;
    unsigned int _raw;
    float _last = 0;

  public:
    Battery(byte pin) {
      this->_pin = pin;
      pinMode(pin, INPUT);
    }

    float level() {
      _raw = analogRead(this->_pin);
      float level = (100 * _raw) / BATT_MAX_RAW;

      if ( this->_last < level && (level - this->_last) > 1 ) {
        this->_last += 1;
      }
      else if ( this->_last > level && (this->_last - level) > 1 ) {
        this->_last -= 1;
      }
      else {
        this->_last = level;
      }

      return this->_last;
    }

};
