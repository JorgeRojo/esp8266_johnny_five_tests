#include "Arduino.h"
#include "./Storage.h"
#include "./Bluetooth.h"
#include "./Orientation.h"
#include "./Battery.h"
#define PIN_BATTERY A0
#define PIN_LED 2


#define PIN_LED_ON 26
#define PIN_LED_BAT_LEVEL_100 18
#define PIN_LED_BAT_LEVEL_075 19
#define PIN_LED_BAT_LEVEL_040 23


Storage storage = Storage(false);
Orientation orientation = Orientation(PIN_LED, storage);
Bluetooth bluetooth = Bluetooth("Polytt_9R4W7bvff9", storage);
Battery battery = Battery(PIN_BATTERY);


char* ant_face = "";





void setup()
{
  Serial.begin(115200);
  delay(500);

  bluetooth.start();
  delay(500);


  orientation.start();
  delay(500);

  //LEDS
  pinMode(PIN_LED_ON, OUTPUT);
  pinMode(PIN_LED_BAT_LEVEL_100, OUTPUT);
  pinMode(PIN_LED_BAT_LEVEL_075, OUTPUT);
  pinMode(PIN_LED_BAT_LEVEL_040, OUTPUT);
}


void loop()
{

  bluetooth.loop();




  orientation.loop();

  char* face = orientation.get_polytt_face();
  if ( face != "" && face != ant_face) {
    Serial.print("FACE\t");
    Serial.println(face);
    ant_face = face;
  }

  //indicator
  if (orientation.state >= 4) {
    digitalWrite(PIN_LED_ON, HIGH);
  }


  //Battery level inidicators
  float batteryLevel = battery.level();
  digitalWrite(PIN_LED_BAT_LEVEL_100, batteryLevel > 90 ? HIGH : LOW);
  digitalWrite(PIN_LED_BAT_LEVEL_075, batteryLevel > 75 ? HIGH : LOW);
  digitalWrite(PIN_LED_BAT_LEVEL_040, batteryLevel > 40 ? HIGH : LOW);

  delay(200);

}
