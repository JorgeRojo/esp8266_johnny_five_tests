#include "Arduino.h"
#include "./Storage.h"
#include "./Battery.h"
#include "./Orientation.h"
#define PIN_BATTERY A0
#define PIN_LED 2


#define PIN_LED_ON 26
#define PIN_LED_BAT_LEVEL_100 18 
#define PIN_LED_BAT_LEVEL_075 19 
#define PIN_LED_BAT_LEVEL_040 23 


Battery battery = Battery(PIN_BATTERY);
Storage storage = Storage(false);
Orientation orientation = Orientation(PIN_LED, storage);



String antFace = "";
String getPolyttFace(float* ypr) {
  
  float mErr = 20;
  String face = "";
  float Y = ypr[1] * 180 / M_PI;
  float X = ypr[2] * 180 / M_PI;
 

  if ( ((40 + mErr) >= Y && (40 - mErr) <= Y) && ((40 + mErr) >= X && (40 - mErr) <= X) ) {
    face = "F";
  }
  if ( ((-40 + mErr) >= Y && (-40 - mErr) <= Y) && ((40 + mErr) >= X && (40 - mErr) <= X) ) {
    face = "E";
  }
  if ( ((-140 + mErr) >= Y && (-140 - mErr) <= Y) && ((140 + mErr) >= X && (140 - mErr) <= X) ) {
    face = "H";
  }
  if ( ((140 + mErr) >= Y && (140 - mErr) <= Y) && ((140 + mErr) >= X && (140 - mErr) <= X) ) {
    face = "G";
  }
  if ( ((140 + mErr) >= Y && (140 - mErr) <= Y) && ((-140 + mErr) >= X && (-140 - mErr) <= X) ) {
    face = "C";
  }
  if ( ((-140 + mErr) >= Y && (-140 - mErr) <= Y) && ((-140 + mErr) >= X && (-140 - mErr) <= X) ) {
    face = "B";
  }
  if ( ((-40 + mErr) >= Y && (-40 - mErr) <= Y) && ((-40 + mErr) >= X && (-40 - mErr) <= X) ) {
    face = "A";
  }
  if ( ((40 + mErr) >= Y && (40 - mErr) <= Y) && ((-40 + mErr) >= X && (-40 - mErr) <= X) ) {
    face = "D";
  }

  return face;
}



void setup()
{
  Serial.begin(115200);
  delay(500);

  orientation.setup();
  delay(500);


  //LEDS 
  pinMode(PIN_LED_ON, OUTPUT);  
  pinMode(PIN_LED_BAT_LEVEL_100, OUTPUT);  
  pinMode(PIN_LED_BAT_LEVEL_075, OUTPUT);  
  pinMode(PIN_LED_BAT_LEVEL_040, OUTPUT);  
}

void loop()
{ 
  // orientation
  orientation.loop();
  float* ypr = orientation.get_ypr();
 
//  Serial.print("YPR\t"); 
//  Serial.print(ypr[1] * 180 / M_PI);
//  Serial.print("\t");
//  Serial.print(ypr[2] * 180 / M_PI);
//  Serial.println("\t");
//  

  String face = getPolyttFace(ypr);
  if( face != "" && face != antFace) { 
    Serial.print("FACE\t");
    Serial.println(getPolyttFace(ypr)); 
    antFace = face;    
  }

  // volts
  float batteryLevel = battery.level();
  Serial.print("Battery llevel: ");
  Serial.println(batteryLevel);


  
  //ready indicator
  if(orientation.state >= 4) {
    digitalWrite(PIN_LED_ON, HIGH);
  }

  //Battery level inidicators
  digitalWrite(PIN_LED_BAT_LEVEL_100, batteryLevel > 90 ? HIGH : LOW);
  digitalWrite(PIN_LED_BAT_LEVEL_075, batteryLevel > 75 ? HIGH : LOW);
  digitalWrite(PIN_LED_BAT_LEVEL_040, batteryLevel > 40 ? HIGH : LOW);

}
