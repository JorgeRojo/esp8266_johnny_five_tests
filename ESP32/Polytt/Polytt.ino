#include "Arduino.h"
#include "./RGBLed.h"
#include "./Storage.h"
#include "./Bluetooth.h"
#include "./Orientation.h"
#include "./Battery.h"

#define PIN_BATTERY A0
#define PIN_LED_RED 25
#define PIN_LED_GREEN 26
#define PIN_LED_BLUE 27

RGBLed rgbled = RGBLed(PIN_LED_RED, PIN_LED_GREEN, PIN_LED_BLUE);
Storage storage = Storage(true);
Orientation orientation = Orientation(rgbled, storage);
Bluetooth bluetooth = Bluetooth("Polytt_9R4W7bvff9", storage);
Battery battery = Battery(PIN_BATTERY);

char *ant_face = "";

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    continue;

  rgbled.start();
  delay(500);

  bluetooth.start();
  delay(500);

  orientation.start();
  delay(500);
}

void loop()
{

  // rgbled.blink(255, 255, 255, 200);

  bluetooth.loop();

  orientation.loop();

  char *face = orientation.get_polytt_face();
  if (face != "" && face != ant_face)
  {
    Serial.print("FACE\t");
    Serial.println(face);
    ant_face = face;
    rgbled.blink(0, 0, 255, 50);
  }

  
  //Battery level inidicators
  float batteryLevel = battery.level();
  Serial.println(batteryLevel);  

  
  if (batteryLevel <= 40)
  {
    rgbled.blink(255, 0, 0, 200);
  } 
  else if (orientation.state >= 4)
  {
    rgbled.blink(255, 255, 255, 50);
  }

}
