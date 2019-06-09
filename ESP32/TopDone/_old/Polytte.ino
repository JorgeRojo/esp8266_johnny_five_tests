#include <I2Cdev.h>
#include <MPU6050_6Axis_MotionApps20.h>
#include <Arduino.h>
#include <Wire.h>
#include <driver/adc.h>
// #include "./Storage.h"

//GND - GND
//VCC - VCC
//SDA - Pin A4
//SCL - Pin A5
//INT - Pin 15
#define _MPU_SIGNAL_PATH_RESET 0x68
#define _MPU_I2C_SLV0_ADDR 0x37
#define _MPU_ACCEL_CONFIG 0x1C
#define _MPU_MOT_THR 0x1F // Motion detection threshold bits [7:0]
#define _MPU_MOT_DUR 0x20 // This seems wrong // Duration counter threshold for motion interrupt generation, 1 kHz rate, LSB = 1 ms
#define _MPU_MOT_DETECT_CTRL 0x69
#define _MPU_INT_ENABLE 0x38
#define _MPU_PWR_MGMT 0x6B //SLEEPY TIME
#define _MPU_INT_STATUS 0x3A
#define _MPU_MPU6050_ADDRESS 0x68 //AD0 is 0

int _mpu_startTime, _mpu_state = 0;
int _mpu_X = -1, _mpu_Y = -1;

// class default I2C address is 0x68
// specific I2C addresses m_mpu_ay be passed as a parameter here
// AD0 low = 0x68
// AD0 high = 0x69
MPU6050 _mpu;
// MPU6050 _mpu(0x69); // <-- use for AD0 high

// MPU control/status vars
bool _mpu_ready = false;
bool _mpu_dmpReady = false;  // set true if DMP init was successful
uint8_t _mpu_IntStatus;   // holds actual interrupt status byte from MPU
uint8_t _mpu_devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t _mpu_packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t _mpu_fifoCount;     // count of all bytes currently in FIFO
uint8_t _mpu_fifoBuffer[64]; // FIFO storage buffer

Quaternion _mpu_q;        // [w, x, y, z]
VectorInt16 _mpu_aa;      // [x, y, z]
VectorInt16 _mpu_aaReal;  // [x, y, z]
VectorInt16 _mpu_aaWorld; // [x, y, z]
VectorFloat _mpu_gravity; // [x, y, z]
float _mpu_ypr[3];        // [yaw, pitch, roll]

void _mpu_writeByte(uint8_t address, uint8_t subAddress, uint8_t data)
{
    Wire.begin();
    Wire.beginTransmission(address); // Initialize the Tx buffer
    Wire.write(subAddress);          // Put slave register address in Tx buffer
    Wire.write(data);                // Put data in Tx buffer
    Wire.endTransmission();          // Send the Tx buffer
}

void _mpu_configuration(int sens)
{
    _mpu_writeByte(_MPU_MPU6050_ADDRESS, 0x6B, 0x00);
    _mpu_writeByte(_MPU_MPU6050_ADDRESS, _MPU_SIGNAL_PATH_RESET, 0x07); //Reset all internal signal paths in the MPU-6050 by writing 0x07 to register 0x68;
    // _mpu_writeByte( _MPU_MPU6050_ADDRESS, _MPU_I2C_SLV0_ADDR, 0x20);//write register 0x37 to select how to use the interrupt pin. For an active high, push-pull signal that st_mpu_ays until register (decimal) 58 is read, write 0x20.
    _mpu_writeByte(_MPU_MPU6050_ADDRESS, _MPU_ACCEL_CONFIG, 0x01);    //Write register 28 (==0x1C) to set the Digital High Pass Filter, bits 3:0. For example set it to 0x01 for 5Hz. (These 3 bits are grey in the data sheet, but they are used! Leaving them 0 means the filter alw_mpu_ays outputs 0.)
    _mpu_writeByte(_MPU_MPU6050_ADDRESS, _MPU_MOT_THR, sens);         //Write the desired Motion threshold to register 0x1F (For example, write decimal 20).
    _mpu_writeByte(_MPU_MPU6050_ADDRESS, _MPU_MOT_DUR, 40);           //Set motion detect duration to 1  ms; LSB is 1 ms @ 1 kHz rate
    _mpu_writeByte(_MPU_MPU6050_ADDRESS, _MPU_MOT_DETECT_CTRL, 0x15); //to register 0x69, write the motion detection decrement and a few other settings (for example write 0x15 to set both free-fall and motion decrements to 1 and accelerometer start-up delay to 5ms total by adding 1ms. )
    _mpu_writeByte(_MPU_MPU6050_ADDRESS, 0x37, 140);             // now INT pin is active low
    _mpu_writeByte(_MPU_MPU6050_ADDRESS, _MPU_INT_ENABLE, 0x40);      //write register 0x38, bit 6 (0x40), to enable motion detection interrupt.
    _mpu_writeByte(_MPU_MPU6050_ADDRESS, _MPU_PWR_MGMT, 8);           // 101000 - Cycle & disable TEMP SENSOR
    _mpu_writeByte(_MPU_MPU6050_ADDRESS, 0x6C, 0x01);            // Disable Gyros
}

void _mpu_enableSleeping() {
    int total = esp_timer_get_time() - _mpu_startTime; 
    _mpu_stopDMP();

    Serial.println("Sleeping");
    adc_power_off(); // adc power off disables wifi entirely, upstream bug
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, 0);
    delay(500); //1 = High, 0 = Low
    adc_power_off();
    esp_deep_sleep_start();
}

void setup()
{

    _mpu_startTime = esp_timer_get_time();

    _mpu_configuration(1);

    // initialize serial communication
    Serial.begin(115200);

    // initialize device
    _mpu.initialize();

    // Comprobar  conexion
    Serial.println("MPU -> Testing device connections...");
    _mpu_ready = _mpu.testConnection();
    Serial.println(_mpu_ready ? "MPU -> connection successful" : "MPU -> connection failed");
}

void loop()
{
    if (_mpu_ready)
    {
        _mpu__mpu_state_0();
        _mpu__mpu_state_1();
        _mpu__mpu_state_2();
        _mpu__mpu_state_3();
        _mpu__mpu_state_4();
        _mpu__mpu_state_5();
    }
}

///////////////////////////////////   MPU    ////////////////////////////////////
int _mpu_buffersize = 1000; //Amount of readings used to average, make it higher to get more precision but sketch will be slower  (default:1000)
int _mpu_acel_deadzone = 8; //Acelerometer error allowed, make it lower to get more precision, but sketch m_mpu_ay not converge  (default:8)
int _mpu_giro_deadzone = 1; //Giro error allowed, make it lower to get more precision, but sketch m_mpu_ay not converge  (default:1)

// _mpu_calibration vars
int16_t _mpu_ax, _mpu_ay, _mpu_az, _mpu_gx, _mpu_gy, _mpu_gz;
int _mpu_mean_ax, _mpu_mean_ay, _mpu_mean_az, _mpu_mean_gx, _mpu_mean_gy, _mpu_mean_gz;
int _mpu_ax_offset, _mpu_ay_offset, _mpu_az_offset, _mpu_gx_offset, _mpu_gy_offset, _mpu_gz_offset;

void _mpu_meansensors()
{
    long i = 0, buff__mpu_ax = 0, buff__mpu_ay = 0, buff__mpu_az = 0, buff__mpu_gx = 0, buff__mpu_gy = 0, buff__mpu_gz = 0;

    while (i < (_mpu_buffersize + 101))
    {
        // read raw accel/_mpu_gyro measurements from device
        _mpu.getMotion6(&_mpu_ax, &_mpu_ay, &_mpu_az, &_mpu_gx, &_mpu_gy, &_mpu_gz);

        if (i > 100 && i <= (_mpu_buffersize + 100))
        { //First 100 measures are discarded
            buff__mpu_ax = buff__mpu_ax + _mpu_ax;
            buff__mpu_ay = buff__mpu_ay + _mpu_ay;
            buff__mpu_az = buff__mpu_az + _mpu_az;
            buff__mpu_gx = buff__mpu_gx + _mpu_gx;
            buff__mpu_gy = buff__mpu_gy + _mpu_gy;
            buff__mpu_gz = buff__mpu_gz + _mpu_gz;
        }
        if (i == (_mpu_buffersize + 100))
        {
            _mpu_mean_ax = buff__mpu_ax / _mpu_buffersize;
            _mpu_mean_ay = buff__mpu_ay / _mpu_buffersize;
            _mpu_mean_az = buff__mpu_az / _mpu_buffersize;
            _mpu_mean_gx = buff__mpu_gx / _mpu_buffersize;
            _mpu_mean_gy = buff__mpu_gy / _mpu_buffersize;
            _mpu_mean_gz = buff__mpu_gz / _mpu_buffersize;
        }
        i++;
        delay(2); //Needed so we don't get repeated measures
    }
}

void _mpu_calibration()
{

    _mpu_ax_offset = -_mpu_mean_ax / 8;
    _mpu_ay_offset = -_mpu_mean_ay / 8;
    _mpu_az_offset = (16384 - _mpu_mean_az) / 8;

    _mpu_gx_offset = -_mpu_mean_gx / 4;
    _mpu_gy_offset = -_mpu_mean_gy / 4;
    _mpu_gz_offset = -_mpu_mean_gz / 4;
    while (1)
    {
        int ready = 0;
        _mpu.setXAccelOffset(_mpu_ax_offset);
        _mpu.setYAccelOffset(_mpu_ay_offset);
        _mpu.setZAccelOffset(_mpu_az_offset);

        _mpu.setXGyroOffset(_mpu_gx_offset);
        _mpu.setYGyroOffset(_mpu_gy_offset);
        _mpu.setZGyroOffset(_mpu_gz_offset);

        _mpu_meansensors();
        Serial.println("...");

        if (abs(_mpu_mean_ax) <= _mpu_acel_deadzone)
            ready++;
        else
            _mpu_ax_offset = _mpu_ax_offset - _mpu_mean_ax / _mpu_acel_deadzone;

        if (abs(_mpu_mean_ay) <= _mpu_acel_deadzone)
            ready++;
        else
            _mpu_ay_offset = _mpu_ay_offset - _mpu_mean_ay / _mpu_acel_deadzone;

        if (abs(16384 - _mpu_mean_az) <= _mpu_acel_deadzone)
            ready++;
        else
            _mpu_az_offset = _mpu_az_offset + (16384 - _mpu_mean_az) / _mpu_acel_deadzone;

        if (abs(_mpu_mean_gx) <= _mpu_giro_deadzone)
            ready++;
        else
            _mpu_gx_offset = _mpu_gx_offset - _mpu_mean_gx / (_mpu_giro_deadzone + 1);

        if (abs(_mpu_mean_gy) <= _mpu_giro_deadzone)
            ready++;
        else
            _mpu_gy_offset = _mpu_gy_offset - _mpu_mean_gy / (_mpu_giro_deadzone + 1);

        if (abs(_mpu_mean_gz) <= _mpu_giro_deadzone)
            ready++;
        else
            _mpu_gz_offset = _mpu_gz_offset - _mpu_mean_gz / (_mpu_giro_deadzone + 1);

        if (ready == 6)
            break;
    }
}

void _mpu_initDMP()
{
    // Iniciar DMP
    Serial.println("MPU -> Initializing DMP...");
    _mpu_devStatus = _mpu.dmpInitialize();

    // Activar DMP
    if (_mpu_devStatus == 0)
    {
        Serial.println("MPU -> Enabling DMP...");
        _mpu.setDMPEnabled(true);

        _mpu_IntStatus = _mpu.getIntStatus();

        Serial.println("MPU -> DMP ready!");
        _mpu_dmpReady = true;

        // get expected DMP packet size for later comparison
        _mpu_packetSize = _mpu.dmpGetFIFOPacketSize();
    }
    else
    {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print("MPU -> DMP Initialization failed (code ");
        Serial.print(_mpu_devStatus);
        Serial.println(" )");

        _mpu_stopDMP();
        _mpu_state = 5;
    }
}

void _mpu_stopDMP()
{
    _mpu.reset();
    _mpu_configuration(1);
}

void _mpu__mpu_state_0()
{
    if (_mpu_state != 0)
        return;
    delay(10);

    // saved _mpu_calibration
    _mpu.setXAccelOffset(1015);
    _mpu.setYAccelOffset(3219);
    _mpu.setZAccelOffset(1807);
    _mpu.setXGyroOffset(-36);
    _mpu.setYGyroOffset(21);
    _mpu.setZGyroOffset(0);
    _mpu_state = 3;
    return;

    Serial.println("MPU -> Reading sensors for first time...");

    // reset offsets
    _mpu.setXAccelOffset(0);
    _mpu.setYAccelOffset(0);
    _mpu.setZAccelOffset(0);
    _mpu.setXGyroOffset(0);
    _mpu.setYGyroOffset(0);
    _mpu.setZGyroOffset(0);

    _mpu_meansensors();
    _mpu_state++;
}

void _mpu__mpu_state_1()
{
    if (_mpu_state != 1)
        return;
    delay(10);
    Serial.println("MPU -> Calculating offsets...");
    _mpu_calibration();
    _mpu_state++;
}

void _mpu__mpu_state_2()
{
    if (_mpu_state != 2)
        return;
    delay(10);

    Serial.println("MPU -> Sensor readings with offsets:\t");
    _mpu_meansensors();
    Serial.print(_mpu_mean_ax);
    Serial.print("\t");
    Serial.print(_mpu_mean_ay);
    Serial.print("\t");
    Serial.print(_mpu_mean_az);
    Serial.print("\t");
    Serial.print(_mpu_mean_gx);
    Serial.print("\t");
    Serial.print(_mpu_mean_gy);
    Serial.print("\t");
    Serial.println(_mpu_mean_gz);
    Serial.print("MPU -> Your offsets:\t");
    Serial.print(_mpu_ax_offset);
    Serial.print("\t");
    Serial.print(_mpu_ay_offset);
    Serial.print("\t");
    Serial.print(_mpu_az_offset);
    Serial.print("\t");
    Serial.print(_mpu_gx_offset);
    Serial.print("\t");
    Serial.print(_mpu_gy_offset);
    Serial.print("\t");
    Serial.println(_mpu_gz_offset);
    Serial.println("\ndata is printed as: acelX acelY acelZ giroX giroY giroZ");
    // Serial.println("Check that your sensor readings are close to 0 0 16384 0 0 0");
    // Serial.println("If _mpu_calibration was succesful write down your offsets so you can set them in your projects using something similar to _mpu.setXAccelOffset(youroffset)");

    _mpu.setXAccelOffset(_mpu_ax_offset);
    _mpu.setYAccelOffset(_mpu_ay_offset);
    _mpu.setZAccelOffset(_mpu_az_offset);
    _mpu.setXGyroOffset(_mpu_gx_offset);
    _mpu.setYGyroOffset(_mpu_gy_offset);
    _mpu.setZGyroOffset(_mpu_gz_offset);

    _mpu_state++;
}

void _mpu__mpu_state_3()
{
    if (_mpu_state != 3)
        return;
    delay(10);

    _mpu_initDMP();

    _mpu_state++;
}

void _mpu__mpu_state_4()
{
    if (!_mpu_dmpReady || _mpu_state != 4)
        return;

    _mpu_IntStatus = _mpu.getIntStatus();

    // Obtener datos del FIFO
    _mpu_fifoCount = _mpu.getFIFOCount();

    // Controlar overflow
    if ((_mpu_IntStatus & 0x10) || _mpu_fifoCount == 1024)
    {
        _mpu.resetFIFO();
        // Serial.println("FIFO overflow!");
    }
    else if (_mpu_IntStatus & 0x02)
    {
        // wait for correct available data length, should be a VERY short wait
        while (_mpu_fifoCount < _mpu_packetSize)
            _mpu_fifoCount = _mpu.getFIFOCount();

        // read a packet from FIFO
        _mpu.getFIFOBytes(_mpu_fifoBuffer, _mpu_packetSize);

        // track FIFO count here in case there is > 1 packet available
        // (this lets us immediately read more without waiting for an interrupt)
        _mpu_fifoCount -= _mpu_packetSize;

        // MMostrar Yaw, Pitch, Roll
        _mpu.dmpGetQuaternion(&_mpu_q, _mpu_fifoBuffer);
        _mpu.dmpGetGravity(&_mpu_gravity, &_mpu_q);
        _mpu.dmpGetYawPitchRoll(_mpu_ypr, &_mpu_q, &_mpu_gravity);

        int thisX = round(_mpu_ypr[1] * 180 / M_PI);
        int thisY = round(_mpu_ypr[2] * 180 / M_PI);

        Serial.print("MPU -> X: ");  Serial.print(_mpu_X);
        Serial.print("\t Y: "); Serial.println(_mpu_Y);
  
        delay(500);
        if(thisX == _mpu_X && thisY == _mpu_Y) {
            _mpu_state++;
        }
        _mpu_X = thisX;
        _mpu_Y = thisY;
    } 
}

void _mpu__mpu_state_5()
{
    if (_mpu_state != 5)
        return; 
    _mpu_enableSleeping();
}