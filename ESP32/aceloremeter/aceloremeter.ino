#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"
#include <EEPROM.h>  

#define INTERRUPT_PIN 32
#define LED 2
 

void setup()
{
	Serial.begin(115200);

	pinMode(LED, OUTPUT);
	pinMode(INTERRUPT_PIN, INPUT);
 
	mpuSetup();
}

void loop()
{
	mpuLoop();
}

void _blink()
{
	int i;
	for (i = 0; i < 10; i = i + 1)
	{
		delay(100);
		digitalWrite(LED, (i % 2 != 0) ? HIGH : LOW);
	}
}

///////////////////////////////////  MPU  ////////////////////////////////////
#pragma region
//GND - GND
//VCC - VCC
//SDA - Pin D21
//SCL - Pin D22
//INT - Pin D7

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68
// AD0 high = 0x69
//MPU6050 mpu(0x69); // <-- use for AD0 high
MPU6050 mpu(0x68);

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;		// return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;	// expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;		// count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

Quaternion q;		 // [w, x, y, z]
VectorInt16 aa;		 // [x, y, z]
VectorInt16 aaReal;  // [x, y, z]
VectorInt16 aaWorld; // [x, y, z]
VectorFloat gravity; // [x, y, z]
float ypr[3];		 // [yaw, pitch, roll]

//////////////   CONFIGURATION   //////////////
//Change this 3 variables if you want to fine tune the skecth to your needs.
int buffersize = 1000; //Amount of readings used to average, make it higher to get more precision but sketch will be slower  (default:1000)
int acel_deadzone = 8; //Acelerometer error allowed, make it lower to get more precision, but sketch may not converge  (default:8)
int giro_deadzone = 1; //Giro error allowed, make it lower to get more precision, but sketch may not converge  (default:1)

int16_t ax, ay, az, gx, gy, gz;
int mean_ax, mean_ay, mean_az, mean_gx, mean_gy, mean_gz, state = 0;
int ax_offset, ay_offset, az_offset, gx_offset, gy_offset, gz_offset = 0;

volatile bool mpuInterrupt = false;

typedef struct
{
  bool saved; 
  int ax_offset; 
  int ay_offset;
  int az_offset; 
  int gx_offset; 
  int gy_offset; 
  int gz_offset; 
} Calibration;


void mpuSetup()
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

	// start message
	Serial.println("\nMPU6050 Calibration Sketch");
	delay(500);
	Serial.println("\nYour MPU6050 should be placed in horizontal position, with package letters facing up. \nDon't touch it until you see a finish message.\n");
	delay(500);

	// Comprobar  conexion
	Serial.println(F("Testing device connections..."));
	Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

	// Iniciar DMP
	Serial.println(F("Initializing DMP..."));
	devStatus = mpu.dmpInitialize();

	// reset offsets
	mpu.setXAccelOffset(0);
	mpu.setYAccelOffset(0);
	mpu.setZAccelOffset(0);
	mpu.setXGyroOffset(0);
	mpu.setYGyroOffset(0);
	mpu.setZGyroOffset(0);

	// Activar DMP
	if (devStatus == 0)
	{
		Serial.println(F("Enabling DMP..."));
		mpu.setDMPEnabled(true);

		// Activar interrupcion
		attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), _dmpDataReady, RISING);
		mpuIntStatus = mpu.getIntStatus();

		Serial.println(F("DMP ready! Waiting for first interrupt..."));
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
		Serial.print(F("DMP Initialization failed (code "));
		Serial.print(devStatus);
		Serial.println(F(")"));
	}
}

void _storeCalibration() 
{
	  
  Calibration calibration;
	calibration.saved = true;
	// calibration.ax_offset = ax_offset;
	// calibration.ay_offset = ay_offset;
	// calibration.az_offset = az_offset;
	// calibration.gx_offset = gx_offset;
	// calibration.gy_offset = gy_offset;
	// calibration.gz_offset = gz_offset;  

	Serial.println("Calibration saved!"); 
	_storeStruct(&calibration, sizeof(calibration));
}

void _storeStruct(void *data_source, size_t size)
{
	EEPROM.begin(size * 2);
	for (size_t i = 0; i < size; i++)
	{
		char data = ((char *)data_source)[i];
		EEPROM.write(i, data);
	}
	EEPROM.commit();
}

void _loadStruct(void *data_dest, size_t size)
{
	EEPROM.begin(size * 2);
	for (size_t i = 0; i < size; i++)
	{
		char data = EEPROM.read(i);
		((char *)data_dest)[i] = data;
	}
}

//void __printStore(&calibration)
//{
//	Serial.print("Saved: ");
//	Serial.print(calibration.saved);
//	Serial.print("\t");
//	Serial.print(calibration.ax_offset);
//	Serial.print("\t");
//	Serial.print(calibration.ay_offset);
//	Serial.print("\t");
//	Serial.print(calibration.az_offset);
//	Serial.print("\t");
//	Serial.print(calibration.gx_offset);
//	Serial.print("\t");
//	Serial.print(calibration.gy_offset);
//	Serial.print("\t");
//	Serial.println(calibration.gz_offset);
//} 

void mpuLoop()
{
	// Si fallo al iniciar, parar programa
	if (!dmpReady)
	{
		return;
	} 

	if (state == 0)
	{
		Serial.println("\nReading sensors and setore...");
		_mpuMeansensors();  

		_blink();
		state++;
		delay(500);
	}

	if (state == 1)
	{
		Serial.println("\nCalculating offsets...");

    Calibration calibration;
    _loadStruct(&calibration, sizeof(calibration));
    
		calibration.saved = false;
		if (!calibration.saved)
		{ 
			_mpuCalibration();
			_storeCalibration();
		}
		else {
			ax_offset = calibration.ax_offset; 
			ay_offset = calibration.ay_offset; 
			az_offset = calibration.az_offset; 
			gx_offset = calibration.gx_offset; 
			gy_offset = calibration.gy_offset; 
			gz_offset = calibration.gz_offset; 
			Serial.println("Calibration recovered from storage!"); 
		}

		state++; 
		_blink();
		delay(500);
	}

	if (state == 2)
	{
		_mpuMeansensors();
		Serial.println("\nFINISHED!");
		Serial.print("\nSensor readings with offsets:\t");
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
		Serial.println("If _mpuCalibration was succesful write down your offsets so you can set them in your projects using something similar to mpu.setXAccelOffset(youroffset)");

		// reset offsets
		mpu.setXAccelOffset(ax_offset);
		mpu.setYAccelOffset(ay_offset);
		mpu.setZAccelOffset(az_offset);
		mpu.setXGyroOffset(gx_offset);
		mpu.setYGyroOffset(gy_offset);
		mpu.setZGyroOffset(gz_offset);

		_blink();
		digitalWrite(LED, LOW);
		state++;
		delay(500);
	}

	if (state == 3)
	{

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
			//Serial.println(F("FIFO overflow!"));
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
			// mpu.dmpGetQuaternion(&q, fifoBuffer);
			// mpu.dmpGetGravity(&gravity, &q);
			// mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
			// Serial.print("ypr\t");
			// Serial.print(ypr[0] * 180 / M_PI);
			// Serial.print("\t");
			// Serial.print(ypr[1] * 180 / M_PI);
			// Serial.print("\t");
			// Serial.println(ypr[2] * 180 / M_PI);

			//Mostrar aceleracion
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

			//      digitalWrite(LED, LOW);
			//      delay(100);
		}
	}
}

void _dmpDataReady()
{
	mpuInterrupt = true;
}

void _mpuMeansensors()
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

		delay(1); //Needed so we don't get repeated measures
	}
}

void _mpuCalibration()
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

		_mpuMeansensors();
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

#pragma endregion
