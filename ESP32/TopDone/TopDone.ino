#include <Arduino.h>
#include "./Storage.h"
#include "./WifiConnect.h"
#include "./BLEConnect.h"
#include "./FaceChanger.h"
#include "./RGBLed.h"

Storage storage = Storage();
RGBLed rgbLed = RGBLed();
WifiConnect wifiConnect = WifiConnect();
BLEConnect bleConnect = BLEConnect();

void handleFaceChange(char *face)
{
    Serial.print(">>>>--FACE--->>>> ");
    Serial.println(face);

    //TODO: send face by wifi endpoint

    if (face != "X")
        rgbLed.blink(0, 255, 0, 10, 60);
    else
        rgbLed.blink(255, 0, 0, 10, 60);
 
}
FaceChanger faceChanger = FaceChanger(handleFaceChange);
 
bool isConnected = false;
void setup()
{
    Serial.begin(115200);

    storage.load(); 


    const char *ssid = storage.data.wifi_ssid;
    const char *pass = storage.data.wifi_pass;
    isConnected = WifiConnect::checkConnection(ssid, pass);

    Serial.print(">>>-- isConnected --->>> ");
    Serial.println(isConnected);

    if (isConnected)
    {
        faceChanger.start();
    }
    else
    {
        bleConnect.start();
    }
}

void loop()
{ 
    if (isConnected)
    {
        faceChanger.loop();
    }
    else
    {
        bleConnect.loop();
    } 
}