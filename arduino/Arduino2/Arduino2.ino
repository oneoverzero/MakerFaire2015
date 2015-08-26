#include <LMotorController.h>
#include <VarSpeedServo.h>

/*
// TODO GET SOFT PWM
// https://code.google.com/p/rogue-code/wiki/SoftPWMLibraryDocumentation
Pinout das ligacoes do Arduino2
Leonardo clone, Arduino Pro Micro

Motor controllers:

LEFT
direction in + /in
A0
A1
enable (PWM)
10

RIGHT
direction in + /in
A2
A3
enable (PWM)
9

Camera turret: (PWM)
3

Wheel servos: (PWM)
5
6

Sobram os pinos :

0 1 2 4 7 8
14 15 16

*/


/* -----------------------------------------------------------------------------------------------------------*/
// Hardware control thing

// motor control pins
int ENA = 10;
int IN1 = A0;
int IN2 = A1;
int IN3 = 9;
int IN4 = A2;
int ENB = A3;
LMotorController motorController(ENA, IN1, IN2, ENB, IN3, IN4, 0.6, 1);

VarSpeedServo topLeft;    // also same as bottomright
VarSpeedServo topRight;   // also same as bottomleft
VarSpeedServo turret;     // servo for camera turret horizontal movement

const int servoTL = 9;   // the digital pin used for the servo
const int servoTR = 10;  // the digital pin used for the servo
const int turretP = 6;   // the digital pin used for the servo


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
const int listSize = 15;
char cmdList[listSize]= {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'};; // the list
char currentCommand;   // active command being executed (check also is we're processingCommands)
int numCommand         = 0;   // command count received
int cmdPointer         = 0;   // helper var for current command in list
int processingCommands = 0;   // flag for helping us know if we're running or stopped
int roverSpeed         = 102; // start it of with 40% power
int isRoverMoving      = 0;   // rover is NOT moving

/* -----------------------------------------------------------------------------------------------------------*/
// loop consts
const int serialMilliesDelay  =  750; // defines for delays for next loop execution
const int commandMilliesDelay =  500;
const int roverMoveMillies    = 2000;
const int roverRotateMillies  = 2000;

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

// clear command list
void resetCmdList() {
  for (int x = 0; x < listSize; x++) {
    cmdList[x] = '0';
  }
}

// process whatever came through serial
void readSerialLine() {
  String inData;
  int x;

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

        numCommand = 0;

        // copy all commands from within the string
        for ( x = 3; x < inData.length() - 3; x++) {
          cmdList[numCommand] = inData.charAt(x);
          numCommand++;
        }
        cmdPointer = 0;
        getNextCommand();
        // get back to rpi2 and say we've parsed this command!
        // TODO
        // Serial.print('GOTMIS');
      }
      // Process inData
      // COL000LOC
      if (inData.startsWith("COL") && inData.endsWith("LOC")) {
        // collision warning
        char fwdleft  = inData.charAt(3);
        char fwdright = inData.charAt(4);
        char backward = inData.charAt(5);

        // invarlidate current command by killing millies
        movingMillies = millis();

        // stop rover
        isRoverMoving = 0;
        motorController.stopMoving();

        // take evasive action with a RED LED
        //setLEDCommandColor(whatever is vermelho)
        if (fwdleft == '1') {
          currentCommand = '5'; // ROR
        } else if (fwdright == '1') {
          currentCommand = '4'; // ROL
        }
        if (backward == '1') {
          currentCommand = '3'; // BCK
        }

        // get back to rpi2 and say we've parsed this command!
        // TODO
        // Serial.print('GOTCOL');
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

// set rover miving millies
void setRoverMillies() {
  movingMillies = millis() + roverMoveMillies;
}

// set LED command color
void setLEDCommandColor(int command) {
  // TODO figure out how to use those colors

  /*
  #define TONES 8
  int RGBColors[TONES][3] = {
  {255, 0, 0}, // Red
  {0, 255, 0}, // Green
  {0, 0, 255}, //Blue
  {255, 255, 0}, // Yellow
  {255, 255, 255}, // White
  {128, 0, 255}, // Purple
  {0, 255, 255}, // Cyan
  {237, 120, 6} // Orange
  };
  void setColor (byte red, byte green, byte blue) {
  // Common Cathode: 255-value
   analogWrite(redPin, 255 - red);
   analogWrite(bluePin, 255 - blue);
   analogWrite(greenPin, 255 - green);
  }

  void setColorIndex(byte index) {
   setColor(RGBColors[index][0], RGBColors[index][1], RGBColors[index][2]);
  }

  */
}

// get next command from queue
void getNextCommand() {
  //
  currentCommand = cmdList[cmdPointer];
  cmdPointer++;
  // if we have reached the last command on the list
  // just sit there, doing NOP and clear command list
  if (cmdPointer > numCommand) {
    currentCommand = 0;
    numCommand = 0;
    resetCmdList();
  }
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
  turret.attach(turretP);

  // TODO setup straigt servo positions
  // depends on hardware setup

  // turn off LED
  setLEDCommandColor(0);

  // set command pointer to 0
  cmdPointer = 0;
  currentCommand = 0;
  numCommand = 0;

}

/* -----------------------------------------------------------------------------------------------------------*/
void loop() {

  // check serial port each serialMillies
  if (millis() >= serialMillies) {
    if (Serial.available() > 0) {
      readSerialLine();
      // wait for next serial port processing
      setSerialMillies();
    }
  }

  // process next command thing
  if (millis() >= nextOutput) {

    // light'em up
    setLEDCommandColor(currentCommand);

    // let us move for these millies
    // can and WILL be cancelled with NOP, STP and others
    setRoverMillies();

    // process currentCommand
    switch (currentCommand) {
      case '0':
        DEBUG_PRINTLN("NOP");
        break;

      case '1':
        DEBUG_PRINTLN("STP");
        // rover is NOT moving, stop both motors
        isRoverMoving = 0;
        motorController.stopMoving();
        break;

      case '2':
        DEBUG_PRINTLN("FWD");
        // only run this bit if not moving
        if (isRoverMoving == 0) {

          // send actual movement command to motors
          // motorcontroller.move(x,y);

          // setup movement flag
          isRoverMoving = 1;
        }
        break;

      case '3':
        DEBUG_PRINTLN("BCK");
        // only run this bit is not moving
        if (isRoverMoving == 0) {
          // let us move for these millies

          // send actual movement command to motors
          // motorcontroller.move(x,y);

          // setup movement flag
          isRoverMoving = 1;
        }
        break;

      case '4':
        DEBUG_PRINTLN("ROL");
        // only run this bit is not moving
        if (isRoverMoving == 0) {
          // let us move for these millies

          // send actual movement command to motors
          // motorcontroller.turnLeft(x,y);

          // setup movement flag
          isRoverMoving = 1;
        }
        break;

      case '5':
        DEBUG_PRINTLN("ROR");
        // only run this bit is not moving
        if (isRoverMoving == 0) {
          // let us move for these millies

          // send actual movement command to motors
          // motorcontroller.turnRight(x,y);

          // setup movement flag
          isRoverMoving = 1;
        }
        break;

    }

    nextOutput = millis() + commandMilliesDelay;
  }

  // is it time to process next command?
  if (isRoverMoving && millis() >= movingMillies) {
    // this command time is done, get next command
    getNextCommand();
  }
}

