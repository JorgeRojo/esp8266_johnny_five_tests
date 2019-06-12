#include <Arduino.h>
#include "./RGBLed.h"
#include "./FaceChanger.h"

RGBLed rgbLed = RGBLed();

void handleFaceChange(char *face)
{
    Serial.print(">>>>--FACE--->>>> ");
    Serial.println(face);
    
    if (face != "X")
    {
        rgbLed.blink(0, 255, 0, 10, 60);
    }
    else
    {
        rgbLed.blink(255, 0, 0, 10, 60);
    }
}

FaceChanger faceChanger = FaceChanger(handleFaceChange);

void setup()
{
    Serial.begin(115200);
    faceChanger.start();
}

void loop()
{
    faceChanger.loop();
}