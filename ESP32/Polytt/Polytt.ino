#include "Arduino.h"
#include "./Storage.h" 
#include "./Battery.h" 
#include "./Orientation.h"  
#define PIN_BATTERY A0 
#define PIN_LED 2  

Storage storage = Storage(true);
Battery battery = Battery(PIN_BATTERY, 4.17, 2);
Orientation orientation = Orientation(PIN_LED, storage);


 

void setup()
{  
	Serial.begin(115200);
	delay(500);
  
	
	orientation.setup(); 
}
  
void loop()
{  
	// volts  
	Serial.print("Volts:\t");
	Serial.print(battery.volt());  

 
	// orientation
  	orientation.loop(); 


	Serial.println("");

	delay(200);

  
}
 

 

 
