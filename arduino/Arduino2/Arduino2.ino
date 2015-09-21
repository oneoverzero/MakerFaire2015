#include <LMotorController.h>
//#include <VarSpeedServo.h>

/*
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
int IN3 = A2;
int IN4 = A3;
int ENB = 9;
LMotorController motor(ENA, IN1, IN2, ENB, IN3, IN4, 0.6, 1);

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

  Navigation parameters from Mission Control
                   Speed in percentages of 255 max value
  A set speed 10%   26
  B set speed 20%   51
  C set speed 40%  102
  D set speed 60%  153
  E set speed 80%  204
  F set speed 100% 255
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
  Y 
  Z 

 */

// command list
const int listSize = 15;
char cmdList[listSize] = {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'};; // the list
char currentCommand;   // active command being executed (check also is we're processingCommands)
int numCommand         = 0;   // command count received
int cmdPointer         = 0;   // helper var for current command in list
int processingCommands = 0;   // flag for helping us know if we're running or stopped
int roverSpeed         = 255; // start it of with 80% power
int isRoverMoving      = 0;   // rover is NOT moving

/* -----------------------------------------------------------------------------------------------------------*/
// loop consts
const int serialMilliesDelay  =  750; // defines for delays for next loop execution
const int commandMilliesDelay = 1000;
const int roverMoveMillies    = 2000;
const int roverRotateMillies  = 1000;

// loop vars
unsigned long serialMillies = 0;    // read serialport each 'x' millies
unsigned long movingMillies = 0;    // move rover for these millies
unsigned long commandMillies = 0;   // move rover for these millies

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
        Serial.print("GOTMIS");
      }
      // Process inData
      // COL000LOC
      if (inData.startsWith("COL") && inData.endsWith("LOC")) {
        // collision warning
        char fwdleft  = inData.charAt(3);
        char fwdright = inData.charAt(4);
        char backward = inData.charAt(5);

        // invalidate current command by killing millies
        movingMillies = millis();

        // stop rover
        isRoverMoving = 0;
        motor.stopMoving();

        // take evasive action with a RED LED
        //setLEDCommandColor(whatever is vermelho)
        if (fwdleft == '1') {
          currentCommand = '5'; // ROR
        } else if (fwdright == '1') {
          currentCommand = '4'; // ROL
        }
        if (backward == '1') {
          currentCommand = '2'; // FWD
        }
        // get back to rpi2 and say we've parsed this command!
        Serial.print('GOTCOL');
      }
      // Process inData
      // CFGABCGFC
      if (inData.startsWith("CFG") && inData.endsWith("GFC")) {

        // invalidate current command by killing millies
        movingMillies = millis();

        // stop rover
        isRoverMoving = 0;
        motor.stopMoving();

        char param1  = inData.charAt(3);

        // process configuration command

        // process currentCommand
        switch (param1) {
          case 'A':
            roverSpeed = 128;
            Serial.println("50%");
            break;
          case 'B':
            roverSpeed = 153;
            Serial.println("60%");
            break;
          case 'C':
            roverSpeed = 178;
            Serial.println("70%");
            break;
          case 'D':
            roverSpeed = 204;
            Serial.println("80%");
            break;
          case 'E':
            roverSpeed = 230;
            Serial.println("90%");
            break;
          case 'F':
            roverSpeed = 255;
            Serial.println("100%");
            break;
        }
        // get back to rpi2 and say we've parsed this command!
        Serial.print('GOTCFG');
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

// set rover miving millies
void setRoverRotateMillies() {
  movingMillies = millis() + roverRotateMillies;
}

// get next command from queue
void getNextCommand() {
  // get next from list
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
  setCommandMillies();
  setSerialMillies();
  setRoverMillies();

  // rover is NOT moving, stop both motors
  isRoverMoving = 0;
  motor.stopMoving();

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
  if (millis() >= commandMillies) {

    // reset next run time
    setCommandMillies();

    // process currentCommand
    switch (currentCommand) {
      case '0':
        Serial.println("NOP");
        break;

      case '1':
        Serial.println("STP");
        // rover is NOT moving, stop both motors
        isRoverMoving = 0;
        motor.stopMoving();
        break;

      case '2':
        Serial.println("FWD");
        // only run this bit if not moving
        if (isRoverMoving == 0) {
          Serial.println("FWD, one time only");
          // let us move for these millies
          // can and WILL be cancelled with NOP, STP and others
          setRoverMillies();

          // send actual movement command to motors
          motor.move(roverSpeed);

          // setup movement flag
          isRoverMoving = 1;
        }
        break;

      case '3':
        Serial.println("BCK");
        // only run this bit is not moving
        if (isRoverMoving == 0) {
          Serial.println("BCK, one time only");
          // let us move for these millies
          // can and WILL be cancelled with NOP, STP and others
          setRoverMillies();

          // send actual movement command to motors
          motor.move(-roverSpeed);

          // setup movement flag
          isRoverMoving = 1;
        }
        break;

      case '4':
        Serial.println("ROL");
        // only run this bit is not moving
        if (isRoverMoving == 0) {
          // let us move for these millies
          // can and WILL be cancelled with NOP, STP and others
          setRoverRotateMillies();

          // send actual movement command to motors
          motor.turnLeft(roverSpeed, true);

          // setup movement flag
          isRoverMoving = 1;
        }
        break;

      case '5':
        Serial.println("ROR");
        // only run this bit is not moving
        if (isRoverMoving == 0) {
          // let us move for these millies
          // can and WILL be cancelled with NOP, STP and others
          setRoverRotateMillies();

          // send actual movement command to motors
          motor.turnRight(roverSpeed, true);

          // setup movement flag
          isRoverMoving = 1;
        }
        break;
      default:
        break;

    }
  }

  // is it time to process next command?
  if (isRoverMoving && millis() >= movingMillies) {
    // this command time is done, get next command
    getNextCommand();
    // clear movement flag
    isRoverMoving = 0;
    // stop motors between commands
    motor.stopMoving();
    // pause for 500mS
    delay(500);
  }
}

