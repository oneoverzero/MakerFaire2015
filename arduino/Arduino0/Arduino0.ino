#include <NewPing.h>
#include <RunningMedian.h>
#include "Adafruit_Sensor.h"

/*

Pinout das ligações do Arduino1
Arduino Redboard clone, Lisbom Mini Maker Faire edition

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

/* -----------------------------------------------------------------------------------------------------------*/
#define SONAR_NUM      3 // Number or sensors.
#define MAX_DISTANCE  40 // Maximum distance (in cm) to ping.
#define PING_INTERVAL 33 // Milliseconds between sensor pings (29ms is about the min to avoid cross-sensor echo).

// Distance sensors
unsigned long pingTimer[SONAR_NUM]; // Holds the times when the next ping should happen for each sensor.
unsigned int cm[SONAR_NUM];         // Where the ping distances are stored.
uint8_t currentSensor = 0;          // Keeps track of which sensor is active.

// loop vars
unsigned long nextOutput = 0;       // when next update should occour
unsigned long loopMillies = 0;

// battery,
int bat1 = 0;
int bat2 = 0;

// use one Running Median Object per sensor
RunningMedian sonar_Median[SONAR_NUM] = {     // Sensor medians object array.
  RunningMedian(3),
  RunningMedian(3),
  RunningMedian(3)
};

NewPing sonar[SONAR_NUM] = {    // Sensor object array.
  NewPing( 7, 7, MAX_DISTANCE), // going for single Arduino pin
  NewPing( 4, 4, MAX_DISTANCE), // as per https://code.google.com/p/arduino-new-ping/wiki/NewPing_Single_Pin_Sketch
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

/* -----------------------------------------------------------------------------------------------------------*/
void setup() {

  // init serial port
  Serial.begin(19200);

  // Distance sensor setup
  pingTimer[0] = millis() + 75;           // First ping starts at 75ms, gives time for the Arduino to chill before starting.
  for (uint8_t i = 0; i < SONAR_NUM; i++) // Set the starting time for each sensor.
    pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;

  // Clear RA objects
  for (uint8_t i = 1; i < SONAR_NUM; i++) // Set the starting time for each sensor.
    sonar_Median[i].clear();

  // next output
  nextOutput = millis() + 250;            // Let's get some samples into Average thing
}

/* -----------------------------------------------------------------------------------------------------------*/
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

  // read battery status
  bat1 = analogRead(A0);
  bat2 = analogRead(A1);

  // next code block will send the whole data packet as one to Raspi2 inner serial port
  // this will be the whole telemetry sensory thing
  if (millis() >= nextOutput) {         // Is it time?
    Serial.print("BEG|");
    for (uint8_t i = 0; i < SONAR_NUM; i++) {
      Serial.print(sonar_Median[i].getMedian());
      Serial.print("|");
    }

	// battery sensing
    Serial.print(bat1);
    Serial.print("|");
    Serial.print(bat2);

    Serial.println("|END");

    nextOutput = millis() + 250;
  }
}

