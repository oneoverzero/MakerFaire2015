#include <NewPing.h>
#include <RunningMedian.h>
#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Adafruit_Sensor.h"
#include <Adafruit_HMC5883_U.h>

/*

Pinout das ligações do Arduino1
Arduino Redboard clone, Lisbom Mini Maker Faire edition

I2C:
A4
A5
2 (INT0) para o Interrupt do MPU6050

BATTERY monitor:
A0
A1

Distance sensors:
4
7
8

Sobram os pinos:

11
A3
A4
3
12
13
5
6
9
10

 */


#define SONAR_NUM      3 // Number or sensors.
#define MAX_DISTANCE  40 // Maximum distance (in cm) to ping.
#define PING_INTERVAL 33 // Milliseconds between sensor pings (29ms is about the min to avoid cross-sensor echo).

// Distance sensors
unsigned long pingTimer[SONAR_NUM]; // Holds the times when the next ping should happen for each sensor.
unsigned int cm[SONAR_NUM];         // Where the ping distances are stored.
uint8_t currentSensor = 0;          // Keeps track of which sensor is active.

// IMU stuff
MPU6050 mpu; // <-- use for AD0 high

// MPU control/status vars
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
//VectorInt16 aa;         // [x, y, z]            accel sensor measurements
//VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
//VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
//float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

// mag stuff
/* Assign a unique ID to this sensor at the same time */
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
float headingDegrees;

// loop vars
unsigned long nextOutput = 0;       // when next update should occour
unsigned long loopMillies = 0;

// INTERRUPT DETECTION ROUTINE

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
  mpuInterrupt = true;
}

// use one Running Median Object per sensor
RunningMedian sonar_Median[SONAR_NUM] = {     // Sensor medians object array.
  RunningMedian(10),
  RunningMedian(10),
  RunningMedian(10)
};

NewPing sonar[SONAR_NUM] = {    // Sensor object array.
  NewPing( 4, 4, MAX_DISTANCE), // going for single Arduino pin
  NewPing( 7, 7, MAX_DISTANCE), // as per https://code.google.com/p/arduino-new-ping/wiki/NewPing_Single_Pin_Sketch
  NewPing( 8, 8, MAX_DISTANCE)
};

void echoCheck() { // If ping received, set the sensor distance to array.
  if (sonar[currentSensor].check_timer())
    cm[currentSensor] = sonar[currentSensor].ping_result / US_ROUNDTRIP_CM;
}

void oneSensorCycle() { // Sensor ping cycle complete, do something with the results.
  for (uint8_t i = 0; i < SONAR_NUM; i++)
    sonar_Median[i].add(cm[i]);    // feed teh troll
}

void gyroOffsets() {
  // supply your own gyro offsets here, scaled for min sensitivity
  mpu.setXGyroOffset(220);
  mpu.setYGyroOffset(76);
  mpu.setZGyroOffset(-85);
  mpu.setZAccelOffset(1788); // 1688 factory default for my test chip
}

float rad2deg(float input) {
  return input * 180 / M_PI;
  //return input;
}


void setup() {

  //TODO should we REALLY need that much checking for I2C devices???

  Serial.begin(57600);

  // Distance sensor setup
  pingTimer[0] = millis() + 75;           // First ping starts at 75ms, gives time for the Arduino to chill before starting.
  for (uint8_t i = 1; i < SONAR_NUM; i++) // Set the starting time for each sensor.
    pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;

  // Clear RA objects
  // TODO: JA Verificar se não tens que chamar o clear a partir do i=0 ?
  for (uint8_t i = 1; i < SONAR_NUM; i++) // Set the starting time for each sensor.
    sonar_Median[i].clear();

  // join I2C bus
  Wire.begin();

  // initialize device
  mpu.initialize();

  // verify connection
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));


  // make sure it worked (returns 0 if so)
  if (mpu.dmpInitialize() == 0) {
    // turn on the DMP, now that it's ready
    gyroOffsets();
    mpu.setDMPEnabled(true);

    // enable Arduino interrupt detection
    attachInterrupt(0, dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
    Serial.print("DMP Initialization failed");
  }

  /* Initialise the sensor */
  if (!mag.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    // TODO: JA Eu não colocava ciclos infinidos. Por e simplemente desligada o codigo que trata disto.
    while (1);
  }

  // next output
  nextOutput = millis() + 250;            // Let's get some samples into Average thing
}

void loop() {

  // Loop through all the sensors.
  for (uint8_t i = 0; i < SONAR_NUM; i++) {
    if (millis() >= pingTimer[i]) {         // Is it this sensor's time to ping?
      pingTimer[i] += PING_INTERVAL * SONAR_NUM;  // Set next time this sensor will be pinged.
      if (i == 0 && currentSensor == SONAR_NUM - 1)
        oneSensorCycle(); // Sensor ping cycle complete, do something with the results.
      sonar[currentSensor].timer_stop();          // Make sure previous timer is canceled before starting a new ping (insurance).
      currentSensor = i;                          // Sensor being accessed.
      cm[currentSensor] = 0;                      // Make distance zero in case there's no ping echo for this sensor.
      sonar[currentSensor].ping_timer(echoCheck); // Do the ping (processing continues, interrupt will call echoCheck to look for echo).
    }
  }

  // wait for MPU interrupt or extra packet(s) available
  while (!mpuInterrupt && fifoCount < packetSize) {
    // other program behavior stuff here
    // .
  }

  // reset interrupt flag and get INT_STATUS byte
  mpuInterrupt = false;
  mpuIntStatus = mpu.getIntStatus();

  // get current FIFO count
  fifoCount = mpu.getFIFOCount();

  // check for overflow (this should never happen unless our code is too inefficient)
  if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
    // reset so we can continue cleanly
    mpu.resetFIFO();
    //Serial.println(F("FIFO overflow!"));

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
  } else if (mpuIntStatus & 0x02) {
    // wait for correct available data length, should be a VERY short wait
    while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

    // read a packet from FIFO
    mpu.getFIFOBytes(fifoBuffer, packetSize);

    // track FIFO count here in case there is > 1 packet available
    // (this lets us immediately read more without waiting for an interrupt)
    fifoCount -= packetSize;

    // display Euler angles in degrees
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

  }

  /* Get a new sensor event */
  sensors_event_t event;
  mag.getEvent(&event);

  // Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  float heading = atan2(event.magnetic.y, event.magnetic.x);

  // Correct for when signs are reversed.
  if (heading < 0)
    heading += 2 * PI;

  // Check for wrap due to addition of declination.
  if (heading > 2 * PI)
    heading -= 2 * PI;

  // Convert radians to degrees for readability.
  headingDegrees = rad2deg(heading);

  // next code block will send the whole data packet as one to Raspi2 inner serial port
  // this will be the whole telemetry sensory thing
  if (millis() >= nextOutput) {         // Is it time?
    Serial.print("BEG|");
    for (uint8_t i = 0; i < SONAR_NUM; i++) {
      Serial.print(sonar_Median[i].getMedian());
      Serial.print("|");
    }
    Serial.print(rad2deg(ypr[0]));
    Serial.print("|");
    Serial.print(rad2deg(ypr[1]));
    Serial.print("|");
    Serial.print(rad2deg(ypr[2]));

    Serial.print("|");
    Serial.print(headingDegrees);
    Serial.println("|END");

    nextOutput = millis() + 250;
  }
}



