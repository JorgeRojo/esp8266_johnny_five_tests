#include <Arduino.h>
#include "./Storage.h"
#include "./RGBLed.h"
#include "./WifiConnect.h"
#include "./FaceChanger.h"
// #include "./BLEConnect.h"

void handleFaceChange(char *face)
{
    wifiConnect.sendFace(face);

    if (face != "X")
        rgbLed.blink(0, 255, 0, 10, 60);
    else
        rgbLed.blink(255, 0, 0, 10, 60);
}

FaceChanger faceChanger = FaceChanger(handleFaceChange);

void setup()
{
    Serial.begin(115200);
    Serial.flush();

    storage.setup();

    wifiConnect.start();

    if (wifiConnect.isWifiConnected())
    {
        faceChanger.start();
    }
    else
    {
        // bleConnect.start();
    }
}

void loop()
{
    // if (wifiConnect.isWifiConnected())
    // {
    //     faceChanger.loop();
    //     wifiConnect.loop();
    // }
    // else
    // {
    //     bleConnect.start();
    // }
}