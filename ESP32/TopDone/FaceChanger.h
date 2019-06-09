#include <Arduino.h>
#include "./Storage.h"
#include <I2Cdev.h>
#include <MPU6050_6Axis_MotionApps20.h>
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

enum class State
{
    mpu_error,
    set_first_offset,
    set_calibration,
    save_calibration,
    init_dmp,
    dmp_error,
    set_measuring,
    measuring_done,
};

class FaceChanger
{
private:
    MPU6050 mpu;
    Storage storage = Storage(false);
    void (*_on_face_change)(char *);

    State state = State::set_first_offset;
    int X = -1;
    int Y = -1;

    // MPU control/status vars
    uint8_t IntStatus;      // holds actual interrupt status byte from MPU
    uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
    uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
    uint16_t fifoCount;     // count of all bytes currently in FIFO
    uint8_t fifoBuffer[64]; // FIFO storage buffer

    Quaternion q;        // [w, x, y, z]
    VectorInt16 aa;      // [x, y, z]
    VectorInt16 aaReal;  // [x, y, z]
    VectorInt16 aaWorld; // [x, y, z]
    VectorFloat gravity; // [x, y, z]
    float ypr[3];        // [yaw, pitch, roll]

    int buffersize = 1000; //Amount of readings used to average, make it higher to get more precision but sketch will be slower  (default:1000)
    int acel_deadzone = 8; //Acelerometer error allowed, make it lower to get more precision, but sketch may not converge  (default:8)
    int giro_deadzone = 1; //Giro error allowed, make it lower to get more precision, but sketch may not converge  (default:1)

    // calibration vars
    int16_t ax, ay, az, gx, gy, gz;
    int mean_ax, mean_ay, mean_az, mean_gx, mean_gy, mean_gz;
    int ax_offset, ay_offset, az_offset, gx_offset, gy_offset, gz_offset;

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

    void meansensors()
    {
        long i = 0, buff_ax = 0, buff_ay = 0, buff_az = 0, buff_gx = 0, buff_gy = 0, buff_gz = 0;

        while (i < (buffersize + 101))
        {
            // read raw accel/gyro measurements from device
            mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

            if (i > 100 && i <= (buffersize + 100))
            { //First 100 measures are discarded
                buff_ax = buff_ax + ax;
                buff_ay = buff_ay + ay;
                buff_az = buff_az + az;
                buff_gx = buff_gx + gx;
                buff_gy = buff_gy + gy;
                buff_gz = buff_gz + gz;
            }
            if (i == (buffersize + 100))
            {
                mean_ax = buff_ax / buffersize;
                mean_ay = buff_ay / buffersize;
                mean_az = buff_az / buffersize;
                mean_gx = buff_gx / buffersize;
                mean_gy = buff_gy / buffersize;
                mean_gz = buff_gz / buffersize;
            }
            i++;
            delay(2); //Needed so we don't get repeated measures
        }
    }

    void calibration()
    {

        ax_offset = -mean_ax / 8;
        ay_offset = -mean_ay / 8;
        az_offset = (16384 - mean_az) / 8;

        gx_offset = -mean_gx / 4;
        gy_offset = -mean_gy / 4;
        gz_offset = -mean_gz / 4;
        while (1)
        {
            int ready = 0;
            mpu.setXAccelOffset(ax_offset);
            mpu.setYAccelOffset(ay_offset);
            mpu.setZAccelOffset(az_offset);
            mpu.setXGyroOffset(gx_offset);
            mpu.setYGyroOffset(gy_offset);
            mpu.setZGyroOffset(gz_offset);

            meansensors();
            Serial.println("...");

            if (abs(mean_ax) <= acel_deadzone)
                ready++;
            else
                ax_offset = ax_offset - mean_ax / acel_deadzone;

            if (abs(mean_ay) <= acel_deadzone)
                ready++;
            else
                ay_offset = ay_offset - mean_ay / acel_deadzone;

            if (abs(16384 - mean_az) <= acel_deadzone)
                ready++;
            else
                az_offset = az_offset + (16384 - mean_az) / acel_deadzone;

            if (abs(mean_gx) <= giro_deadzone)
                ready++;
            else
                gx_offset = gx_offset - mean_gx / (giro_deadzone + 1);

            if (abs(mean_gy) <= giro_deadzone)
                ready++;
            else
                gy_offset = gy_offset - mean_gy / (giro_deadzone + 1);

            if (abs(mean_gz) <= giro_deadzone)
                ready++;
            else
                gz_offset = gz_offset - mean_gz / (giro_deadzone + 1);

            if (ready == 6)
                break;
        }
    }

    void stopDMP()
    {
        mpu.reset();
        configuration(1);
    }

    char * getFace()
    {
        float e = 30; // error margin
        char *face = "X";

        if (((0 + e) >= Y && (0 - e) <= Y) && ((0 + e) >= X && (0 - e) <= X))
        {
            face = "A";
        }
        if (((-30 + e) >= Y && (-30 - e) <= Y) && ((-60 + e) >= X && (-60 - e) <= X))
        {
            face = "B";
        }
        if ((((180 + e) >= Y && (180 - e) <= Y) || ((-180 + e) >= Y && (-180 - e) <= Y)) && ((-120 + e) >= X && (-120 - e) <= X))
        {
            face = "C";
        }
        if (((60 + e) >= Y && (60 - e) <= Y) && ((0 + e) >= X && (0 - e) <= X))
        {
            face = "D";
        }
        if (((-120 + e) >= Y && (-120 - e) <= Y) && (((180 + e) >= X && (180 - e) <= X) || ((-180 + e) >= X && (-180 - e) <= X)))
        {
            face = "E";
        }
        if (((0 + e) >= Y && (0 - e) <= Y) && ((60 + e) >= X && (60 - e) <= X))
        {
            face = "F";
        }
        if ((((180 + e) >= Y && (180 - e) <= Y) || ((-180 + e) >= Y && (-180 - e) <= Y)) && (((180 + e) >= X && (180 - e) <= X) || ((-180 + e) >= X && (-180 - e) <= X)))
        {
            face = "H";
        }
        if (((120 + e) >= Y && (120 - e) <= Y) && ((120 + e) >= X && (120 - e) <= X))
        {
            face = "G";
        }

        return face;
    }

    // states ---------------------
    void setFirstOffset()
    {
        if (state != State::set_first_offset)
            return;
 
        if (storage.data.calibration_saved)
        {
            Serial.println("MPU -> Set offset from storeage");

            mpu.setXAccelOffset(storage.data.ax_offset);
            mpu.setYAccelOffset(storage.data.ay_offset);
            mpu.setZAccelOffset(storage.data.az_offset);
            mpu.setXGyroOffset(storage.data.gx_offset);
            mpu.setYGyroOffset(storage.data.gy_offset);
            mpu.setZGyroOffset(storage.data.gz_offset);

            state = State::init_dmp;
        }
        else
        {

            Serial.println("MPU -> Reset offset");

            // reset offsets
            mpu.setXAccelOffset(0);
            mpu.setYAccelOffset(0);
            mpu.setZAccelOffset(0);
            mpu.setXGyroOffset(0);
            mpu.setYGyroOffset(0);
            mpu.setZGyroOffset(0);

            meansensors();

            state = State::set_calibration;
        }

        delay(10);
    }

    void setCalibration()
    {
        if (state != State::set_calibration)
            return;

        Serial.println("MPU -> Calculating offsets...");
        calibration();

        mpu.setXAccelOffset(ax_offset);
        mpu.setYAccelOffset(ay_offset);
        mpu.setZAccelOffset(az_offset);
        mpu.setXGyroOffset(gx_offset);
        mpu.setYGyroOffset(gy_offset);
        mpu.setZGyroOffset(gz_offset);

        state = State::save_calibration;
        delay(10);
    }

    void saveCalibration()
    {
        if (state != State::save_calibration)
            return;

        storage.data.calibration_saved = true;
        storage.data.ax_offset = ax_offset;
        storage.data.ay_offset = ay_offset;
        storage.data.az_offset = az_offset;
        storage.data.gx_offset = gx_offset;
        storage.data.gy_offset = gy_offset;
        storage.data.gz_offset = gz_offset;
        storage.save(true);

        state = State::init_dmp;

        delay(10);
    }

    void initDMP()
    {

        if (state != State::init_dmp)
            return;

        // Iniciar DMP
        Serial.println("MPU -> Initializing DMP...");
        devStatus = mpu.dmpInitialize();

        // Activar DMP
        if (devStatus == 0)
        {
            Serial.println("MPU -> Enabling DMP...");
            mpu.setDMPEnabled(true);

            IntStatus = mpu.getIntStatus();

            Serial.println("MPU -> DMP ready!");

            // get expected DMP packet size for later comparison
            packetSize = mpu.dmpGetFIFOPacketSize();

            state = State::set_measuring;
        }
        else
        {
            // ERROR!
            // 1 = initial memory load failed
            // 2 = DMP configuration updates failed
            // (if it's going to break, usually the code will be 1)
            Serial.print("MPU -> DMP Initialization failed (code ");
            Serial.print(devStatus);
            Serial.println(" )");

            state = State::dmp_error; 
            
            ESP.restart();
        }

        delay(10);
    }

    void setMeasuring()
    {
        if (state != State::set_measuring)
            return;

        IntStatus = mpu.getIntStatus();

        // Obtener datos del FIFO
        fifoCount = mpu.getFIFOCount();

        // Controlar overflow
        if ((IntStatus & 0x10) || fifoCount == 1024)
        {
            mpu.resetFIFO();
            // Serial.println("FIFO overflow!");
        }
        else if (IntStatus & 0x02)
        {
            // wait for correct available data length, should be a VERY short wait
            while (fifoCount < packetSize)
                fifoCount = mpu.getFIFOCount();

            // read a packet from FIFO
            mpu.getFIFOBytes(fifoBuffer, packetSize);

            // track FIFO count here in case there is > 1 packet available
            // (this lets us immediately read more without waiting for an interrupt)
            fifoCount -= packetSize;

            // MMostrar Yaw, Pitch, Roll
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

            int thisX = round(ypr[1] * 180 / M_PI);
            int thisY = round(ypr[2] * 180 / M_PI);

            Serial.print("MPU -> X: ");
            Serial.print(X);
            Serial.print("\t Y: ");
            Serial.println(Y);

            delay(100);
            if (thisX == X && thisY == Y)
            {
                state = State::measuring_done;
                stopDMP();
            }
            X = thisX;
            Y = thisY;
        }
    }

    void triggerFaceChange()
    {

        if (state != State::measuring_done)
            return;

        if (_on_face_change)
        {
            _on_face_change(getFace());
            sleep();
        }
 
    } 

    void sleep()
    {
        Serial.println("MPU -> Sleeping");

        adc_power_off(); // adc power off disables wifi entirely, upstream bug
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, 0);
        delay(500); //1 = High, 0 = Low
        adc_power_off();
        esp_deep_sleep_start();
    }

public:
    FaceChanger(void (*on_face_change)(char *))
    {
        _on_face_change = on_face_change;
    }

    void start()
    {
        storage.load();

        configuration(1);

        // initialize device
        mpu.initialize();

        // Comprobar  conexion
        Serial.println("MPU -> Testing device connections...");

        if (!mpu.testConnection())
        {
            state = State::dmp_error;
            Serial.println("MPU -> connection failed");
        }
        else
        {
            Serial.println("MPU -> connection successful");
        }
    }

    void loop()
    {
        setFirstOffset();
        setCalibration();
        saveCalibration();
        initDMP();
        setMeasuring();
        triggerFaceChange();
    }


};