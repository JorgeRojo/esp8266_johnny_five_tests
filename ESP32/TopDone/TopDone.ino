#include <Arduino.h>
#include "./FaceChanger.h"
 

void handleFaceChange(char * face) {
    Serial.print(">>>>--FACE--->>>> ");
    Serial.println(face);
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