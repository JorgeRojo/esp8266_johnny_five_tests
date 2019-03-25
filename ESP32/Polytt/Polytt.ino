#include "Arduino.h"
#include "./Storage.h"
#include "./RGBLed.h"
#include "./Bluetooth.h"
#include "./Orientation.h"
#include "./Battery.h"
#include "./WifiNotifier.h"

#define PIN_BATTERY A0
#define PIN_LED_RED 23
#define PIN_LED_GREEN 18
#define PIN_LED_BLUE 19

RGBLed rgbled = RGBLed(PIN_LED_RED, PIN_LED_GREEN, PIN_LED_BLUE);
Storage storage = Storage(false);
Battery battery = Battery(PIN_BATTERY, storage);
Orientation orientation = Orientation(rgbled, storage);
Bluetooth bluetooth = Bluetooth("Polytt_9R4W7bvff9", storage);

char *ant_face = "";

void setup()
{
    Serial.begin(115200);
    while (!Serial) {
        continue; 
        delay(500);
    }
        
    delay(500);
    
    Serial.println();
    Serial.println("************** POLYTT START **************");
    Serial.println();

    battery.start();  
    rgbled.start(); 
    bluetooth.start();  
    orientation.start();
    
    delay(500);
}

void loop()
{ 
  
    bluetooth.loop();

    orientation.loop();

    char *face = orientation.get_polytt_face();
    if (face != "X" && face != ant_face)
    {
        Serial.print("MPU -> FACE\t");
        Serial.println(face);
        ant_face = face;
    }
 

    //Battery level inidicators
    float batteryLevel = battery.level();
    //Serial.println(batteryLevel);

    if (batteryLevel <= 40) {
        rgbled.color(255, 0, 0);
    }

    delay(200);
}
