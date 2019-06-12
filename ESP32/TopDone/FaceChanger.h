#include <Arduino.h>
#include <MPU6050_tockn.h>
#include <Wire.h>
#include <driver/adc.h>
#include "./Storage.h"

//GND - GND
//VCC - VCC
//SDA - Pin A4
//SCL - Pin A5
//INT - Pin 15

MPU6050 mpu6050(Wire);

class FaceChanger
{
private:
    Storage storage = Storage(false);
    void (*_on_face_change)(char *);
    int timeStart = 0;

    int X = 0, Y = 0, preX = 999, preY = 999;
    char *prevFace = "";

    bool coordMatch(int y_angle, int x_angle)
    {
        float e = 40; // error margin
        bool match = false;
        bool y_match = ((y_angle + e) >= Y && (y_angle - e) <= Y);
        if (y_angle == 180 || y_angle == -180)
        {
            y_match = y_match || ((-y_angle + e) >= Y && (-y_angle - e) <= Y);
        }
        bool x_match = ((x_angle + e) >= X && (x_angle - e) <= X);
        if (x_match == 180 || x_match == -180)
        {
            x_match = x_match || ((-x_angle + e) >= X && (-x_angle - e) <= X);
        }

        match = y_match && x_match;

        if (match)
        {
            // Serial.print("MPU F -> Y: ");
            // Serial.print(y_angle);
            // Serial.print("\t X: ");
            // Serial.println(x_angle);
        }

        return match;
    }

    char *getFace()
    {
        char *face = "X";

        if (coordMatch(0, -60))
        {
            face = "A";
        }
        if (coordMatch(160, -120))
        {
            face = "B";
        }
        if (coordMatch(120, -160))
        {
            face = "C";
        }
        if (coordMatch(-60, 0))
        {
            face = "D";
        }
        if (coordMatch(-120, -160))
        {
            face = "E";
        }
        if (coordMatch(0, 60))
        {
            face = "F";
        }
        if (coordMatch(160, 120))
        {
            face = "H";
        }
        if (coordMatch(60, 0))
        {
            face = "G";
        }

        return face;
    }

    void sleep()
    {
        int total = round((millis() - timeStart) / 1000);

        if (timeStart > 0 && total > 1)
        {
            Serial.println("MPU -> Sleeping");

            adc_power_off(); // adc power off disables wifi entirely, upstream bug
            esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, 0);
            delay(100); //1 = High, 0 = Low
            adc_power_off();
            esp_deep_sleep_start();
        }
    }

public:
    FaceChanger(void (*on_face_change)(char *))
    {
        _on_face_change = on_face_change;
    }

    void start()
    {
        storage.load();

        Wire.begin();
        mpu6050.begin();

        if (storage.data.mpu6050_saved)
        {
            mpu6050.setGyroOffsets(
                storage.data.mpu6050_x_offset,
                storage.data.mpu6050_y_offset,
                storage.data.mpu6050_z_offset);
        }
        else
        {
            mpu6050.calcGyroOffsets(true);
            storage.data.mpu6050_saved = true;
            storage.data.mpu6050_x_offset = mpu6050.getGyroXoffset();
            storage.data.mpu6050_y_offset = mpu6050.getGyroYoffset();
            storage.data.mpu6050_z_offset = mpu6050.getGyroZoffset();
            storage.save();
        }
    }

    void loop()
    {
        mpu6050.update();
        preY = round(mpu6050.getAngleY());
        preX = round(mpu6050.getAngleX());

        delay(10);

        if (preY != Y || preX != X) {
            timeStart = millis();
        }

        Y = preY;
        X = preX;

        char *face = getFace();

        if (_on_face_change && prevFace != face)
        {
            _on_face_change(face);
            prevFace = face;
        }

        sleep();
    }
};