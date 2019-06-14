#include <Arduino.h> 
#include "./FaceChanger.h"
#include "./RGBLed.h"

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

    rgbLed.black();
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