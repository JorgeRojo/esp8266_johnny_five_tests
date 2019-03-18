#define BATT_MAX_RAW 2100

class Battery {

  private:
    byte _pin;
    unsigned int _raw;
    float _last = BATT_MAX_RAW;
    int _r = 1;

  public:
    Battery(byte pin) {
      this->_pin = pin;
      pinMode(pin, INPUT);
    }

    float level() {
      _raw = analogRead(this->_pin);

      if ( this->_last < _raw && (_raw - this->_last) > this->_r) {
        this->_last += this->_r;
      }
      else if ( this->_last > _raw && (this->_last - _raw) > this->_r) {
        this->_last -= this->_r;
      }
      else {
        this->_last = _raw;
      }

          
      float level = (100 * this->_last) / BATT_MAX_RAW; 

      return level;
    }

};
