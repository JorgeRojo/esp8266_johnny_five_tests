//GND - GND
//VCC - VCC
//SDA - Pin A4
//SCL - Pin A5
//INT - Pin 15
#define SIGNAL_PATH_RESET 0x68
#define I2C_SLV0_ADDR 0x37
#define ACCEL_CONFIG 0x1C
#define MOT_THR 0x1F // Motion detection threshold bits [7:0]
#define MOT_DUR 0x20 // This seems wrong // Duration counter threshold for motion interrupt generation, 1 kHz rate, LSB = 1 ms
#define MOT_DETECT_CTRL 0x69
#define INT_ENABLE 0x38
#define PWR_MGMT 0x6B //SLEEPY TIME
#define INT_STATUS 0x3A
#define MPU6050_ADDRESS 0x68 //AD0 is 0
#define INTERRUPT_PIN 15 

#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68
// AD0 high = 0x69
MPU6050 mpu;
// MPU6050 mpu(0x69); // <-- use for AD0 high

// MPU control/status vars
bool mpuReady = false;
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


void writeByte(uint8_t address, uint8_t subAddress, uint8_t data)
{ 
    Wire.begin();
    Wire.beginTransmission(address); // Initialize the Tx buffer
    Wire.write(subAddress);          // Put slave register address in Tx buffer
    Wire.write(data);                // Put data in Tx buffer
    Wire.endTransmission();          // Send the Tx buffer
} 

uint8_t readByte(uint8_t address, uint8_t subAddress)
{
    uint8_t data;                          // `data` will store the register data
    Wire.beginTransmission(address);       // Initialize the Tx buffer
    Wire.write(subAddress);                // Put slave register address in Tx buffer
    Wire.endTransmission(false);           // Send the Tx buffer, but send a restart to keep connection alive
    Wire.requestFrom(address, (uint8_t)1); // Read one byte from slave register address
    data = Wire.read();                    // Fill Rx buffer with result
    return data;                           // Return data read from slave register
}

void mpuConfiguration(int sens)
{ 
    writeByte(MPU6050_ADDRESS, 0x6B, 0x00);
    writeByte(MPU6050_ADDRESS, SIGNAL_PATH_RESET, 0x07); //Reset all internal signal paths in the MPU-6050 by writing 0x07 to register 0x68;
    writeByte( MPU6050_ADDRESS, I2C_SLV0_ADDR, 0x20);//write register 0x37 to select how to use the interrupt pin. For an active high, push-pull signal that stays until register (decimal) 58 is read, write 0x20.
    writeByte(MPU6050_ADDRESS, ACCEL_CONFIG, 0x01);    //Write register 28 (==0x1C) to set the Digital High Pass Filter, bits 3:0. For example set it to 0x01 for 5Hz. (These 3 bits are grey in the data sheet, but they are used! Leaving them 0 means the filter always outputs 0.)
    writeByte(MPU6050_ADDRESS, MOT_THR, sens);         //Write the desired Motion threshold to register 0x1F (For example, write decimal 20).
    writeByte(MPU6050_ADDRESS, MOT_DUR, 40);           //Set motion detect duration to 1  ms; LSB is 1 ms @ 1 kHz rate
    writeByte(MPU6050_ADDRESS, MOT_DETECT_CTRL, 0x15); //to register 0x69, write the motion detection decrement and a few other settings (for example write 0x15 to set both free-fall and motion decrements to 1 and accelerometer start-up delay to 5ms total by adding 1ms. )
    writeByte(MPU6050_ADDRESS, 0x37, 140);             // now INT pin is active low
    writeByte(MPU6050_ADDRESS, INT_ENABLE, 0x40);      //write register 0x38, bit 6 (0x40), to enable motion detection interrupt.
    writeByte(MPU6050_ADDRESS, PWR_MGMT, 8);           // 101000 - Cycle & disable TEMP SENSOR
    writeByte(MPU6050_ADDRESS, 0x6C, 7);               // Disable Gyros
}


volatile bool mpuInterrupt = false;
void dmpDataReady()
{
    mpuInterrupt = true;
}

void waitForReady()
{
    // wait for ready
    while (Serial.available() && Serial.read())
        ; // empty buffer
    while (!Serial.available())
    {
        Serial.println(F("Send any character to start sketch.\n"));
        delay(1500);
    }
    while (Serial.available() && Serial.read())
        ; // empty buffer again
}

void setup()
{

    mpuConfiguration(1);

    // initialize serial communication
    Serial.begin(115200);

    // initialize device
    mpu.initialize();

    // wait for ready
    waitForReady();
 
    pinMode(INTERRUPT_PIN, INPUT);

    // Comprobar  conexion
    Serial.println("Testing device connections...");
    mpuReady = mpu.testConnection();
    Serial.println( mpuReady 
        ? "MPU6050 connection successful" 
        : "MPU6050 connection failed" 
    );  

    if(!mpuReady) return;

    _mpu_state_0();
    _mpu_state_1();
    _mpu_state_2();
    _mpu_state_3();
    _mpu_state_4();

}

void loop()
{ 
}



///////////////////////////////////   MPU CALIBRATION   ////////////////////////////////////
int buffersize = 1000; //Amount of readings used to average, make it higher to get more precision but sketch will be slower  (default:1000)
int acel_deadzone = 8; //Acelerometer error allowed, make it lower to get more precision, but sketch may not converge  (default:8)
int giro_deadzone = 1; //Giro error allowed, make it lower to get more precision, but sketch may not converge  (default:1)

// calibration vars
int16_t ax, ay, az, gx, gy, gz;
int mean_ax, mean_ay, mean_az, mean_gx, mean_gy, mean_gz, state = 0;
int ax_offset, ay_offset, az_offset, gx_offset, gy_offset, gz_offset;

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

void _mpu_state_0()
{ 
    if (state != 0) return;
    Serial.println("Reading sensors for first time...");

    // reset offsets
    mpu.setXAccelOffset(0);
    mpu.setYAccelOffset(0);
    mpu.setZAccelOffset(0);
    mpu.setXGyroOffset(0);
    mpu.setYGyroOffset(0);
    mpu.setZGyroOffset(0);  

    meansensors();
    state++;
    delay(500);
}

void _mpu_state_1()
{ 
    if (state != 1) return;
    Serial.println("Calculating offsets...");
    calibration();
    state++;
    delay(500);
}

void _mpu_state_2()
{ 
    if (state != 2) return;

    Serial.println("Sensor readings with offsets:\t");
    meansensors();
    Serial.print(mean_ax);
    Serial.print("\t");
    Serial.print(mean_ay);
    Serial.print("\t");
    Serial.print(mean_az);
    Serial.print("\t");
    Serial.print(mean_gx);
    Serial.print("\t");
    Serial.print(mean_gy);
    Serial.print("\t");
    Serial.println(mean_gz);
    Serial.print("Your offsets:\t");
    Serial.print(ax_offset);
    Serial.print("\t");
    Serial.print(ay_offset);
    Serial.print("\t");
    Serial.print(az_offset);
    Serial.print("\t");
    Serial.print(gx_offset);
    Serial.print("\t");
    Serial.print(gy_offset);
    Serial.print("\t");
    Serial.println(gz_offset);
    Serial.println("\nData is printed as: acelX acelY acelZ giroX giroY giroZ");
    Serial.println("Check that your sensor readings are close to 0 0 16384 0 0 0");
    Serial.println("If calibration was succesful write down your offsets so you can set them in your projects using something similar to mpu.setXAccelOffset(youroffset)");

    state++;
    delay(500);
}
 
void _mpu_state_3()
{ 
    if (state != 3) return; 
 
    // Iniciar DMP
    Serial.println( "Seting offsets and Initializing DMP..." );
    devStatus = mpu.dmpInitialize();

    mpu.setXAccelOffset(ax_offset);
    mpu.setYAccelOffset(ay_offset);
    mpu.setZAccelOffset(az_offset);
    mpu.setXGyroOffset(gx_offset);
    mpu.setYGyroOffset(gy_offset);
    mpu.setZGyroOffset(gz_offset); 

     // Activar DMP
    if (devStatus == 0)
    {
        Serial.println(F("Enabling DMP..."));
        mpu.setDMPEnabled(true);

        // Activar interrupcion
        attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();

        Serial.println("DMP ready! Waiting for first interrupt...");
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    }
    else
    {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print("DMP Initialization failed (code ");
        Serial.print(devStatus);
        Serial.println(" )");
    }

    state++;
    delay(500);
}

void _mpu_state_4()
{ 
    if (state != 4 || !dmpReady)
        return;

    // Ejecutar mientras no hay interrupcion
    while (!mpuInterrupt && fifoCount < packetSize)
    {
        // AQUI EL RESTO DEL CODIGO DE TU PROGRRAMA
    }

    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();

    // Obtener datos del FIFO
    fifoCount = mpu.getFIFOCount();

    // Controlar overflow
    if ((mpuIntStatus & 0x10) || fifoCount == 1024)
    {
        mpu.resetFIFO();
        Serial.println(F("FIFO overflow!"));
    }
    else if (mpuIntStatus & 0x02)
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
        // Serial.print("ypr\t");
        // Serial.print(ypr[0] * 180 / M_PI);
        Serial.print("X: ");
        Serial.print(ypr[1] * 180 / M_PI);
        Serial.print("\t Y: ");
        Serial.println(ypr[2] * 180 / M_PI);

        // Mostrar aceleracion
        // mpu.dmpGetQuaternion(&q, fifoBuffer);
        // mpu.dmpGetAccel(&aa, fifoBuffer);
        // mpu.dmpGetGravity(&gravity, &q);
        // mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
        // Serial.print("areal\t");
        // Serial.print(aaReal.x);
        // Serial.print("\t");
        // Serial.print(aaReal.y);
        // Serial.print("\t");
        // Serial.println(aaReal.z);
    }
}