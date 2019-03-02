#include "Arduino.h"
#include "./Storage.h"
#include "./Battery.h"
#include "./Orientation.h"
#define PIN_BATTERY A0
#define PIN_LED 2

Battery battery = Battery(PIN_BATTERY);
Storage storage = Storage(false);
Orientation orientation = Orientation(PIN_LED, storage);

void setup()
{
  Serial.begin(115200);
  delay(500);

  orientation.setup();
  delay(500);
}

void loop()
{

  // orientation
  orientation.loop();
  float* ypr = orientation.get_ypr();

  Serial.print("YPR\t");
  Serial.print(ypr[0] * 180 / M_PI);
  Serial.print("\t");
  Serial.print(ypr[1] * 180 / M_PI);
  Serial.print("\t");
  Serial.print(ypr[2] * 180 / M_PI);
  Serial.print("\t\t");


  // volts
  float level = battery.level();
  Serial.println(level);
  delay(100);
}
