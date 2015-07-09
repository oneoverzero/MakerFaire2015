#include <LMotorController.h>

// motor control pins
int ENA = 3;
int IN1 = 4;
int IN2 = 8;
int IN3 = 5;
int IN4 = 7;
int ENB = 6;

LMotorController motorController(ENA, IN1, IN2, ENB, IN3, IN4, 0.6, 1);

// loop vars
unsigned long nextOutput = 0;       // when next update should occour
unsigned long loopMillies = 0;


void setup() {

  //TODO should we REALLY need that much checking for I2C devices???
  
  Serial.begin(115200);

  // next output
  nextOutput = millis() + 250;            // Let's get some samples into Average thing
}

void loop() {


  // next code block will send the whole data packet as one to Raspi2 inner serial port
  // this will be the whole telemetry sensory thing
  if (millis() >= nextOutput) {         // Is it time?

    Serial.print("|");
    

    nextOutput = millis() + 250;
  }
}



