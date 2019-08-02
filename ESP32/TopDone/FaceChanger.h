#include <Arduino.h>
#include <MPU6050_tockn.h>
#include <Wire.h>
#include <driver/adc.h>

//GND - GND
//VCC - VCC
//SDA - Pin A4
//SCL - Pin A5
//INT - Pin 15
#define FC_SIGNAL_PATH_RESET 0x68
#define FC_I2C_SLV0_ADDR 0x37
#define FC_ACCEL_CONFIG 0x1C
#define FC_MOT_THR 0x1F // Motion detection threshold bits [7:0]
#define FC_MOT_DUR 0x20 // This seems wrong // Duration counter threshold for motion interrupt generation, 1 kHz rate, LSB = 1 ms
#define FC_MOT_DETECT_CTRL 0x69
#define FC_INT_ENABLE 0x38
#define FC_PWR_MGMT 0x6B //SLEEPY TIME
#define FC_INT_STATUS 0x3A
#define FC_MPU6050_ADDRESS 0x68 //AD0 is 0

MPU6050 mpu6050(Wire);

class FaceChanger
{
private:
    void (*_on_face_change)(char *);
    int timeStart = 0;

    int X = 0, Y = 0, preX = 999, preY = 999;
    char *prevFace = "";
    bool moveEnd = false;

    void writeByte(uint8_t address, uint8_t subAddress, uint8_t data)
    {
        Wire.begin();
        Wire.beginTransmission(address); // Initialize the Tx buffer
        Wire.write(subAddress);          // Put slave register address in Tx buffer
        Wire.write(data);                // Put data in Tx buffer
        Wire.endTransmission();          // Send the Tx buffer
    }

    void configuration(int sens)
    {
        writeByte(FC_MPU6050_ADDRESS, 0x6B, 0x00);
        writeByte(FC_MPU6050_ADDRESS, FC_SIGNAL_PATH_RESET, 0x07); //Reset all internal signal paths in the MPU-6050 by writing 0x07 to register 0x68;
        // writeByte( FC_MPU6050_ADDRESS, FC_I2C_SLV0_ADDR, 0x20);//write register 0x37 to select how to use the interrupt pin. For an active high, push-pull signal that stays until register (decimal) 58 is read, write 0x20.
        writeByte(FC_MPU6050_ADDRESS, FC_ACCEL_CONFIG, 0x01);    //Write register 28 (==0x1C) to set the Digital High Pass Filter, bits 3:0. For example set it to 0x01 for 5Hz. (These 3 bits are grey in the data sheet, but they are used! Leaving them 0 means the filter always outputs 0.)
        writeByte(FC_MPU6050_ADDRESS, FC_MOT_THR, sens);         //Write the desired Motion threshold to register 0x1F (For example, write decimal 20).
        writeByte(FC_MPU6050_ADDRESS, FC_MOT_DUR, 40);           //Set motion detect duration to 1  ms; LSB is 1 ms @ 1 kHz rate
        writeByte(FC_MPU6050_ADDRESS, FC_MOT_DETECT_CTRL, 0x15); //to register 0x69, write the motion detection decrement and a few other settings (for example write 0x15 to set both free-fall and motion decrements to 1 and accelerometer start-up delay to 5ms total by adding 1ms. )
        writeByte(FC_MPU6050_ADDRESS, 0x37, 140);                // now INT pin is active low
        writeByte(FC_MPU6050_ADDRESS, FC_INT_ENABLE, 0x40);      //write register 0x38, bit 6 (0x40), to enable motion detection interrupt.
        writeByte(FC_MPU6050_ADDRESS, FC_PWR_MGMT, 8);           // 101000 - Cycle & disable TEMP SENSOR
        writeByte(FC_MPU6050_ADDRESS, 0x6C, 0x01);               // Disable Gyros
    }

    bool coordMatch(int y_angle, int x_angle)
    {
        float e = 30; // error margin
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

        return match;
    }

    char *getFace()
    {
        char *face = "X";

        if (coordMatch(45, 0))
        {
            face = "A";
        }
        if (coordMatch(0, 45))
        {
            face = "B";
        }
        if (coordMatch(-45, 0))
        {
            face = "C";
        }
        if (coordMatch(0, -45))
        {
            face = "D";
        }
        if (coordMatch(-160, -120))
        {
            face = "E";
        }
        if (coordMatch(-120, -160))
        {
            face = "F";
        }
        if (coordMatch(120, 170))
        {
            face = "H";
        }
        if (coordMatch(-160, 120))
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
        Serial.println("MPU -> start");

        Wire.begin();
        mpu6050.begin();

        configuration(1);

        Storage_t data = storage.getData();

        if (data.mpuSaved)
        {
            mpu6050.setGyroOffsets(
                data.mpuX,
                data.mpuY,
                data.mpuZ);
        }
        else
        {
            mpu6050.calcGyroOffsets(true);
            storage.saveMpu(
                mpu6050.getGyroXoffset(),
                mpu6050.getGyroYoffset(),
                mpu6050.getGyroZoffset());
        }
    }

    void loop()
    {
        mpu6050.update();
        preY = round(mpu6050.getAngleY());
        preX = round(mpu6050.getAngleX());

        delay(10);

        if (preY != Y || preX != X)
        {
            timeStart = millis();
            moveEnd = false;
        }
        else
        {
            moveEnd = true;
        }

        Y = preY;
        X = preX;

        char *face = getFace();

        if (_on_face_change && moveEnd && prevFace != face)
        {
            Serial.printf("Y %s X %s \n", Y, X);

            _on_face_change(face);
            prevFace = face;
        }

        sleep();
    }
};
