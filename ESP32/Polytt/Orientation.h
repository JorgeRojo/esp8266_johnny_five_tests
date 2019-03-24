#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"

//GND - GND
//VCC - VCC
//SDA - Pin D21
//SCL - Pin D22

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68
// AD0 high = 0x69
//MPU6050 mpu(0x69); // <-- use for AD0 high
MPU6050 mpu(0x68);

class Orientation
{
  private:
    // MPU control/status vars
    bool dmpReady = false;  // set true if DMP init was successful
    uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
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

    //////////////   CONFIGURATION   //////////////
    //Change this 3 variables if you want to fine tune the skecth to your needs.
    int buffersize = 1000; //Amount of readings used to average, make it higher to get more precision but sketch will be slower  (default:1000)
    int acel_deadzone = 8; //Acelerometer error allowed, make it lower to get more precision, but sketch may not converge  (default:8)
    int giro_deadzone = 1; //Giro error allowed, make it lower to get more precision, but sketch may not converge  (default:1)

    int16_t ax, ay, az, gx, gy, gz;
    int mean_ax, mean_ay, mean_az, mean_gx, mean_gy, mean_gz;
    int ax_offset, ay_offset, az_offset, gx_offset, gy_offset, gz_offset = 0;

    RGBLed rgbled = RGBLed(0, 0, 0);
    Storage _storage = Storage(false);

    void blink()
    {
        int i;
        for (i = 0; i < 10; i = i + 1)
        {
            if(i % 2 != 0) { 
                this->rgbled.white(); 
            } 
            else {
                this->rgbled.black();  
            }
            delay(100);
        }
    }

    void mpuMeansensors()
    {
        long i = 0, buff_ax = 0, buff_ay = 0, buff_az = 0, buff_gx = 0, buff_gy = 0, buff_gz = 0;

        while (i < (this->buffersize + 101))
        {
            // read raw accel/gyro measurements from device
            mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

            if (i > 100 && i <= (this->buffersize + 100))
            { //First 100 measures are discarded
                buff_ax = buff_ax + ax;
                buff_ay = buff_ay + ay;
                buff_az = buff_az + az;
                buff_gx = buff_gx + gx;
                buff_gy = buff_gy + gy;
                buff_gz = buff_gz + gz;
            }
            if (i == (this->buffersize + 100))
            {
                mean_ax = buff_ax / this->buffersize;
                mean_ay = buff_ay / this->buffersize;
                mean_az = buff_az / this->buffersize;
                mean_gx = buff_gx / this->buffersize;
                mean_gy = buff_gy / this->buffersize;
                mean_gz = buff_gz / this->buffersize;
            }
            i++;

            delay(1); //Needed so we don't get repeated measures
        }
    }

    void mpuCalibration()
    {
        this->ax_offset = -this->mean_ax / 8;
        this->ay_offset = -this->mean_ay / 8;
        this->az_offset = (16384 - this->mean_az) / 8;

        this->gx_offset = -this->mean_gx / 4;
        this->gy_offset = -this->mean_gy / 4;
        this->gz_offset = -this->mean_gz / 4;
        while (1)
        {
            int ready = 0;
            mpu.setXAccelOffset(this->ax_offset);
            mpu.setYAccelOffset(this->ay_offset);
            mpu.setZAccelOffset(this->az_offset);

            mpu.setXGyroOffset(this->gx_offset);
            mpu.setYGyroOffset(this->gy_offset);
            mpu.setZGyroOffset(this->gz_offset);

            mpuMeansensors();
            Serial.println("...");

            if (abs(this->mean_ax) <= this->acel_deadzone)
                ready++;
            else
                this->ax_offset = this->ax_offset - this->mean_ax / this->acel_deadzone;

            if (abs(this->mean_ay) <= this->acel_deadzone)
                ready++;
            else
                this->ay_offset = this->ay_offset - this->mean_ay / this->acel_deadzone;

            if (abs(16384 - this->mean_az) <= this->acel_deadzone)
                ready++;
            else
                this->az_offset = this->az_offset + (16384 - this->mean_az) / this->acel_deadzone;

            if (abs(this->mean_gx) <= this->giro_deadzone)
                ready++;
            else
                this->gx_offset = this->gx_offset - this->mean_gx / (this->giro_deadzone + 1);

            if (abs(this->mean_gy) <= this->giro_deadzone)
                ready++;
            else
                this->gy_offset = this->gy_offset - this->mean_gy / (this->giro_deadzone + 1);

            if (abs(this->mean_gz) <= this->giro_deadzone)
                ready++;
            else
                this->gz_offset = this->gz_offset - this->mean_gz / (this->giro_deadzone + 1);

            if (ready == 6)
                break;
        }
    }

    void _state_0()
    {
        if (this->state == 0)
        {

            Serial.println("MPU -> Reading sensors...");
            mpuMeansensors();

            //load setted data
            this->_storage.load(false);

            this->state++;
            this->blink();
        }
    }
    void _state_1()
    {
        if (this->state == 1)
        {

            if (!this->_storage.data.calibration_saved)
            {
                Serial.println("MPU -> Calculating offsets...");
                mpuCalibration();
            }
            else
            {
                Serial.println("MPU -> Setting offsets from storage...");
                this->ax_offset = this->_storage.data.ax_offset;
                this->ay_offset = this->_storage.data.ay_offset;
                this->az_offset = this->_storage.data.az_offset;
                this->gx_offset = this->_storage.data.gx_offset;
                this->gy_offset = this->_storage.data.gy_offset;
                this->gz_offset = this->_storage.data.gz_offset;
                delay(1000);
            }

            this->state++;
            this->blink();
        }
    }
    void _state_2()
    {
        if (this->state == 2)
        {
            mpuMeansensors();

            Serial.print("MPU -> Sensor readings with offsets:\t");
            Serial.print(this->mean_ax);
            Serial.print("\t");
            Serial.print(this->mean_ay);
            Serial.print("\t");
            Serial.print(this->mean_az);
            Serial.print("\t");
            Serial.print(this->mean_gx);
            Serial.print("\t");
            Serial.print(this->mean_gy);
            Serial.print("\t");
            Serial.println(this->mean_gz);

            Serial.print("MPU -> Your offsets:\t");
            Serial.print(this->ax_offset);
            Serial.print("\t");
            Serial.print(this->ay_offset);
            Serial.print("\t");
            Serial.print(this->az_offset);
            Serial.print("\t");
            Serial.print(this->gx_offset);
            Serial.print("\t");
            Serial.print(this->gy_offset);
            Serial.print("\t");
            Serial.println(this->gz_offset);
            Serial.println("MPU -> Data is printed as: acelX acelY acelZ giroX giroY giroZ");

            mpu.setXAccelOffset(this->ax_offset);
            mpu.setYAccelOffset(this->ay_offset);
            mpu.setZAccelOffset(this->az_offset);
            mpu.setXGyroOffset(this->gx_offset);
            mpu.setYGyroOffset(this->gy_offset);
            mpu.setZGyroOffset(this->gz_offset);

            this->state++;
            this->blink();
        }
    }
    void _state_3()
    {
        if (this->state == 3)
        {

            // Saving calibration
            if (!this->_storage.data.calibration_saved)
            { 
                this->_storage.data.calibration_saved = true; 
                this->_storage.data.ax_offset = this->ax_offset;
                this->_storage.data.ay_offset = this->ay_offset;
                this->_storage.data.az_offset = this->az_offset;
                this->_storage.data.gx_offset = this->gx_offset;
                this->_storage.data.gy_offset = this->gy_offset;
                this->_storage.data.gz_offset = this->gz_offset;
                this->_storage.save();
            }

            this->blink();
            this->state++;
        }
    }
    void _state_4()
    {
        if (this->state == 4)
        {

            this->rgbled.color(0, 0, 0);

            mpuIntStatus = mpu.getIntStatus();

            // Obtener datos del FIFO
            this->fifoCount = mpu.getFIFOCount();

            // Controlar overflow
            if ((mpuIntStatus & 0x10) || this->fifoCount == 1024)
            {
                mpu.resetFIFO();
                //          Serial.println(F("FIFO overflow!"));
            }
            else if (mpuIntStatus & 0x02)
            {
                // wait for correct available data length, should be a VERY short wait
                while (this->fifoCount < this->packetSize)
                {
                    this->fifoCount = mpu.getFIFOCount();
                }

                // read a packet from FIFO
                mpu.getFIFOBytes(this->fifoBuffer, this->packetSize);

                // track FIFO count here in case there is > 1 packet available
                // (this lets us immediately read more without waiting for an interrupt)
                this->fifoCount -= this->packetSize;

                //Yaw, Pitch, Roll
                mpu.dmpGetQuaternion(&this->q, this->fifoBuffer);
                mpu.dmpGetGravity(&this->gravity, &this->q);
                mpu.dmpGetYawPitchRoll(this->ypr, &this->q, &this->gravity);
                //                    Serial.print("YPR\t");
                //                    Serial.print(this->ypr[0] * 180 / M_PI);
                //                    Serial.print("\t");
                //                    Serial.print(this->ypr[1] * 180 / M_PI);
                //                    Serial.print("\t");
                //                    Serial.print(this->ypr[2] * 180 / M_PI);

                //Acceleration
                mpu.dmpGetQuaternion(&this->q, this->fifoBuffer);
                mpu.dmpGetAccel(&this->aa, this->fifoBuffer);
                mpu.dmpGetGravity(&this->gravity, &this->q);
                mpu.dmpGetLinearAccel(&this->aaReal, &this->aa, &this->gravity);
                //                    Serial.print("\tAREAL\t");
                //                    Serial.print(this->aaReal.x);
                //                    Serial.print("\t");
                //                    Serial.print(this->aaReal.y);
                //                    Serial.print("\t");
                //                    Serial.print(this->aaReal.z);
                //
                //                    Serial.println("");
            }
        }
    }

  public:
    Orientation(RGBLed rgbled, Storage &storage)
    {
        this->rgbled = rgbled;
        this->_storage = storage;
    }

    bool on = false;
    int state = 0;
    void start()
    {

        // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
#endif

        // initialize device
        mpu.initialize();

        // connection test
        Serial.println(F("MPU -> Testing device connections..."));
        Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

        // initialice DMP
        Serial.println(F("MPU -> Initializing DMP..."));
        this->devStatus = mpu.dmpInitialize();

        // reset offsets
        mpu.setXAccelOffset(0);
        mpu.setYAccelOffset(0);
        mpu.setZAccelOffset(0);
        mpu.setXGyroOffset(0);
        mpu.setYGyroOffset(0);
        mpu.setZGyroOffset(0);

        // Enabling DMP
        if (this->devStatus == 0)
        {
            Serial.println(F("MPU -> Enabling DMP..."));
            mpu.setDMPEnabled(true);

            mpuIntStatus = mpu.getIntStatus();

            this->dmpReady = true;

            // get expected DMP packet size for later comparison
            this->packetSize = mpu.dmpGetFIFOPacketSize();
        }
        else
        {
            // ERROR!
            // 1 = initial memory load failed
            // 2 = DMP configuration updates failed
            // (if it's going to break, usually the code will be 1)
            Serial.print(F("MPU -> DMP Initialization failed (code "));
            Serial.print(devStatus);
            Serial.println(F(")"));
        }

        this->on = true;
    }

    void loop()
    {
        if (!this->dmpReady)
        {
            return;
        }

        switch (this->state)
        {
        case 0:
            this->_state_0();
            break;
        case 1:
            this->_state_1();
            break;
        case 2:
            this->_state_2();
            break;
        case 3:
            this->_state_3();
            break;
        case 4:
            this->_state_4();
            break;
        default:
            Serial.println("MPU -> State exceeded!!");
            break;
        }
    }

    float *get_ypr()
    {
        return this->ypr;
    }

    VectorInt16 get_aaReal()
    {
        return this->aaReal;
    }

    char *get_polytt_face()
    {

        float m_err = 30;
        char *face = "X";
        float Y = this->ypr[1] * 180 / M_PI;
        float X = this->ypr[2] * 180 / M_PI;
 

        if (((0 + m_err) >= Y && (0 - m_err) <= Y) && ((0 + m_err) >= X && (0 - m_err) <= X))
        {
            face = "A";
        }
        if (((-60 + m_err) >= Y && (-60 - m_err) <= Y) && ((-60 + m_err) >= X && (-60 - m_err) <= X))
        {
            face = "B";
        }
        if ( (((180 + m_err) >= Y && (180 - m_err) <= Y) || ((-180 + m_err) >= Y && (-180 - m_err) <= Y))
          && ((-120 + m_err) >= X && (-120 - m_err) <= X)
        )
        {
            face = "C";
        }
        if (((60 + m_err) >= Y && (60 - m_err) <= Y) && ((-60 + m_err) >= X && (-60 - m_err) <= X))
        {
            face = "D";
        }
        if (((-120 + m_err) >= Y && (-120 - m_err) <= Y) && ((120 + m_err) >= X && (120 - m_err) <= X))
        {
            face = "E";
        }
        if (((0 + m_err) >= Y && (0 - m_err) <= Y) && ((60 + m_err) >= X && (60 - m_err) <= X))
        {
            face = "F";
        }
         if ( (((180 + m_err) >= Y && (180 - m_err) <= Y) || ((-180 + m_err) >= Y && (-180 - m_err) <= Y))
          && (((180 + m_err) >= X && (180 - m_err) <= X) || ((-180 + m_err) >= X && (-180 - m_err) <= X))
        )
        {
            face = "H";
        }
        if (((120 + m_err) >= Y && (120 - m_err) <= Y) && ((120 + m_err) >= X && (120 - m_err) <= X))
        {
            face = "G";
        } 

        return face;
    }
};
