#include <LMotorController.h>
#include <VarSpeedServo.h>

/* -----------------------------------------------------------------------------------------------------------*/
// Hardware control thing

// motor control pins
int ENA = 3;
int IN1 = 4;
int IN2 = 8;
int IN3 = 5;
int IN4 = 7;
int ENB = 6;
LMotorController motorController(ENA, IN1, IN2, ENB, IN3, IN4, 0.6, 1);

VarSpeedServo topLeft;    // also same as bottomright
VarSpeedServo topRight;   // also same as bottomleft

const int servoTL = 9;  // the digital pin used for the servo
const int servoTR = 9;  // the digital pin used for the servo


/* -----------------------------------------------------------------------------------------------------------*/

/*
  Accepted commands from Mission Control

  0 NOP do nothing
  1 STP STOP!
  2 FWD move ForWarD
  3 BCK move BaCKwards
  4 ROL rotate LEFT
  5 ROR rotate RIGHT
  6 P1C take picture with main camera (centered)
  7 PAN take 3 pictures with main camera, produce a panoramic image
  8 LAS extend robotic arm and use laser
  9 P2C extend robotic arm and take picture with auxiliary camera

  Navigation parameters from Mission Control
                   Speed in percentages of 255 max value
  A set speed 10%   26
  B set speed 20%   51
  C set speed 40%  102
  D set speed 60%  153
  E set speed 80%  204
  F
  G
  H
  I
  J
  K
  L
  M
  N
  O
  P
  Q
  R
  S
  T
  U
  V avoid right collision, rotate LEFT
  W avoid left collision, rotate RIGHT
  X set distance to avoid Collision
  Y Set overall speed (10 to 90%)
  Z NOP do nothing

 */

// command list
#define listSize 15
int cmdList[listSize]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int nextCommand        = 0;
int roverSpeed         = 102; // start it of with 40% power
int isRoverMoving      = 0;   // rover is NOT moving

/* -----------------------------------------------------------------------------------------------------------*/
// loop consts
#define serialMilliesDelay    750
#define commandMilliesDelay   500


// loop vars
unsigned long nextOutput = 0;       // when next update should occour
unsigned long loopMillies = 0;

unsigned long serialMillies = 0;    // read serialport each 'x' millies
unsigned long movingMillies = 0;    // move rover for these millies
unsigned long commandMillies = 0;   // move rover for these millies

/* -----------------------------------------------------------------------------------------------------------*/
// DEBUG stuff
#define DEBUG_ME

#ifdef DEBUG
#define DEBUG_PRINT(x)    Serial.print (x)
#define DEBUG_PRINTDEC(x) Serial.print (x, DEC)
#define DEBUG_PRINTLN(x)  Serial.println (x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTDEC(x)
#define DEBUG_PRINTLN(x)
#endif

/* -----------------------------------------------------------------------------------------------------------*/

void resetCmdList() {
  for (int x = 0; x <= listSize; x++) {
    cmdList[x] = 0;
  }
}

void readSerialLine() {
  String inData;
  int x, y;

  while (Serial.available() > 0)
  {
    char received = Serial.read();
    inData += received;

    // Process message when new line character is received
    if (received == '\n') {
      // Process inData
      // MIS0123456789XYZSIM
      inData.trim();

      if (inData.startsWith("MIS") && inData.endsWith("SIM")) {
        // clear previous list
        resetCmdList();

        // copy all commands from within the string
        for ( x = 3; x < inData.length() - 3; x++) {
          cmdList[x - 3] = inData.charAt(x) - '0';
        }
        nextCommand = cmdList[0];
      }
      // clear received buffer
      inData = ""; 
    }
  }
}

// set next milles serial port should be read
void setSerialMillies() {
  serialMillies = millis() + serialMilliesDelay;
}

// set next milles commmand processing
void setCommandMillies() {
  commandMillies = millis() + commandMilliesDelay;
}


/* -----------------------------------------------------------------------------------------------------------*/

void setup() {

  // initialize serial port
  Serial.begin(19200);

  // clear command list
  resetCmdList();

  // next output
  nextOutput = millis() + commandMilliesDelay;
  setSerialMillies();
  setCommandMillies();

  // rover is NOT moving, stop both motors
  isRoverMoving = 0;
  motorController.stopMoving();

  // initialize servos
  topLeft.attach(servoTL);
  topRight.attach(servoTR);

}

/* -----------------------------------------------------------------------------------------------------------*/

void loop() {

  // check serial port each serialMillies
  if (millis() >= serialMillies) {
    if (Serial.available() > 0) {
      readSerialLine();
      setSerialMillies();
    }
  }


  // process next command thing
  if (millis() >= nextOutput) {

    switch (nextCommand) {
      case 0:
        DEBUG_PRINTLN("NOP");
        break;
      case 1:
        DEBUG_PRINTLN("STP");
        break;
      case 2:
        DEBUG_PRINTLN("FWD");
        break;
      case 3:
        DEBUG_PRINTLN("BCK");
        break;
    }




    // debug commandList
    for (int x = 0; x <= listSize; x++) {
      DEBUG_PRINT(cmdList[x]);
    }
    DEBUG_PRINTLN("");

    nextOutput = millis() + commandMilliesDelay;
  }
}
