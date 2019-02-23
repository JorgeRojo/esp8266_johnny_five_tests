#include "Arduino.h"
#include "./Storage.h" 
#include "./Battery.h" 
#include "./Orientation.h"  
#define PIN_BATTERY A0 
#define PIN_LED 2  

Storage storage = Storage(false);
Battery battery = Battery(PIN_BATTERY, 4.17, 2);
Orientation orientation = Orientation(PIN_LED, storage);


 

void setup()
{  
	Serial.begin(9600);
	delay(500);
	
	orientation.setup(); 
}
  
void loop()
{  
	// orientation
	orientation.loop(); 

	// volts  
	Serial.print("Volts:\t");
	Serial.print(battery.volt());  
	Serial.println("");
}
 

 

 
