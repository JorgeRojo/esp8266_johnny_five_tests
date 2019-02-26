class Battery { 
	private:
		byte _pin;
		unsigned int _raw;
		float _max; 
		float _res;

	public:
		Battery(byte pin, float batteryMaxVolt, float resistanceDivider) {
			this->_pin = pin;
			this->_max = batteryMaxVolt;
			this->_res = resistanceDivider; 
			pinMode(pin, INPUT);
		} 

		float volt() {     
      		_raw = float(analogRead(this->_pin)); 
			return ((this->_max / this->_res) * (_raw / 1023));
		}  
};
 
