#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"
#include <AccelStepper.h>

// SoftwareSerial is used to communicate with the XBee
#include <SoftwareSerial.h>

int targetSpeed[] = {0, 0};
int targetDir[] = {FORWARD, FORWARD};
const int SPEED_INC = 10;
boolean updateSpeed = false;
boolean updateDir = false;
boolean justStarted = true;
unsigned long int lastMoveTime = 0;
const int UPDATE_THRESH = 10000; //  Start a new move every 10 seconds so we don't accidentally stop

unsigned long int lastPrintTime = 0;
const int PRINT_THRESH = 2000; //  Start a new move every 10 seconds so we don't accidentally stop

Adafruit_MotorShield AFMS = Adafruit_MotorShield();   
Adafruit_StepperMotor *motor[] = {AFMS.getStepper(200, 1), AFMS.getStepper(200, 2)};  

SoftwareSerial XBee(2, 3); // Arduino RX, TX (XBee Dout, Din)

void setup()
{
  // Initialize XBee Software Serial port. Make sure the baud
  // rate matches your XBee setting (9600 is default).
  Serial.begin(9600);
  XBee.begin(9600); 
  printMenu(); // Print a helpful menu:

}

void loop()
{
  // In loop() we continously check to see if a command has been
  //  received.
  if (XBee.available())
  {
    char c = XBee.read();
    switch (c)
       {
    case 's':
    case 'S':
      Serial.println("Adjusting speed!");
      adjustSpeed();  // Change speed of requested motor
      break;
    case 'f':
    case 'F':
      Serial.println("Flipping motor!");
      flipDir();  // Flip direction of requested motor
      break;
    case 'w':      // If received 'w'
    case 'W':      // or 'W'
      writeAPin(); // Write analog pin
      break;
    case 'd':      // If received 'd'
    case 'D':      // or 'D'
      Serial.println("Writing to digital pin");
      writeDPin(); // Write digital pin
      break;
    case 'r':      // If received 'r'
    case 'R':      // or 'R'
      readDPin();  // Read digital pin
      break;
    case 'a':      // If received 'a'
    case 'A':      // or 'A'
      readAPin();  // Read analog pin
      break;
    }    
  }

  if(millis() - lastPrintTime > PRINT_THRESH){
      for(int i = 0; i < 2; i++){    
        XBee.print("Motor ");
        XBee.print(i);
        XBee.print(" speed: ");
        XBee.print(targetSpeed[i]);
        XBee.print(" dir: ");
        XBee.println(targetDir[i]);
      }
      lastPrintTime = millis();
      justStarted = false;
    }
}

void flipDir()
{  
  while (XBee.available() < 1)
    ; // Wait for pin and value to become available
  int motor = ASCIItoInt(XBee.read()); 
  flipDir(motor);
}

void flipDir(int which){
  if(targetDir[which] == 2)
    targetDir[which] = 1;
  else
    targetDir[which] = 2;

  targetSpeed[which] *= -1;
  updateDir = true;

  XBee.print("Flipping motor ");
  XBee.print(which);
  XBee.println(" direction");  
}

void adjustSpeed()
{  
  Serial.println("Waiting for bits!");
  while (XBee.available() < 2)
    ; // Wait for pin and value to become available
  Serial.println("Got the speed info!");
  char motor = ASCIItoInt(XBee.read());
  char upDown = ASCIItoHL(XBee.read());
  
  adjustSpeed(motor, upDown);
}

void adjustSpeed(int which, boolean increase){
  Serial.print("Adjusting motor ");
  Serial.println(which);
  if(increase)
    targetSpeed[which] += SPEED_INC;
  else
    targetSpeed[which] -= SPEED_INC;
    
  if(targetSpeed[which] < 0 && targetDir[which] != 2){
    targetDir[which] = 2;
    updateDir = true;
  }
  else if(targetSpeed[which] >= 0 && targetDir[which] != 1){
    targetDir[which] = 1;
    updateDir = true;
  }  
  updateSpeed = true;

  XBee.print("New motor ");
  XBee.print(which);
  XBee.print("speed: ");
  XBee.println(targetSpeed[which]);
}

// Write Digital Pin
// Send a 'd' or 'D' to enter.
// Then send a pin #
//   Use numbers for 0-9, and hex (a, b, c, or d) for 10-13
// Then send a value for high or low
//   Use h, H, or 1 for HIGH. Use l, L, or 0 for LOW
void writeDPin()
{
  while (XBee.available() < 2)
    ; // Wait for pin and value to become available
  char pin = XBee.read();
  char hl = ASCIItoHL(XBee.read());

  // Print a message to let the control know of our intentions:
  XBee.print("Setting pin ");
  XBee.print(pin);
  XBee.print(" to ");
  XBee.println(hl ? "HIGH" : "LOW");

  pin = ASCIItoInt(pin); // Convert ASCCI to a 0-13 value
  pinMode(pin, OUTPUT); // Set pin as an OUTPUT
  digitalWrite(pin, hl); // Write pin accordingly
}

// Write Analog Pin
// Send 'w' or 'W' to enter
// Then send a pin #
//   Use numbers for 0-9, and hex (a, b, c, or d) for 10-13
//   (it's not smart enough (but it could be) to error on
//    a non-analog output pin)
// Then send a 3-digit analog value.
//   Must send all 3 digits, so use leading zeros if necessary.
void writeAPin()
{
  while (XBee.available() < 4)
    ; // Wait for pin and three value numbers to be received
  char pin = XBee.read(); // Read in the pin number
  int value = ASCIItoInt(XBee.read()) * 100; // Convert next three
  value += ASCIItoInt(XBee.read()) * 10;     // chars to a 3-digit
  value += ASCIItoInt(XBee.read());          // number.
  value = constrain(value, 0, 255); // Constrain that number.

  // Print a message to let the control know of our intentions:
  XBee.print("Setting pin ");
  XBee.print(pin);
  XBee.print(" to ");
  XBee.println(value);

  pin = ASCIItoInt(pin); // Convert ASCCI to a 0-13 value
  pinMode(pin, OUTPUT); // Set pin as an OUTPUT
  analogWrite(pin, value); // Write pin accordingly
}

// Read Digital Pin
// Send 'r' or 'R' to enter
// Then send a digital pin # to be read
// The Arduino will print the digital reading of the pin to XBee.
void readDPin()
{
  while (XBee.available() < 1)
    ; // Wait for pin # to be available.
  char pin = XBee.read(); // Read in the pin value

  // Print beggining of message
  XBee.print("Pin ");
  XBee.print(pin);

  pin = ASCIItoInt(pin); // Convert pin to 0-13 value
  pinMode(pin, INPUT); // Set as input
  // Print the rest of the message:
  XBee.print(" = "); 
  XBee.println(digitalRead(pin));
}

// Read Analog Pin
// Send 'a' or 'A' to enter
// Then send an analog pin # to be read.
// The Arduino will print the analog reading of the pin to XBee.
void readAPin()
{
  while (XBee.available() < 1)
    ; // Wait for pin # to be available
  char pin = XBee.read(); // read in the pin value

  // Print beginning of message
  XBee.print("Pin A");
  XBee.print(pin);

  pin = ASCIItoInt(pin); // Convert pin to 0-6 value
  // Printthe rest of the message:
  XBee.print(" = ");
  XBee.println(analogRead(pin));
}

// ASCIItoHL
// Helper function to turn an ASCII value into either HIGH or LOW
int ASCIItoHL(char c)
{
  // If received 0, byte value 0, L, or l: return LOW
  // If received 1, byte value 1, H, or h: return HIGH
  if ((c == '0') || (c == 0) || (c == 'L') || (c == 'l'))
    return LOW;
  else if ((c == '1') || (c == 1) || (c == 'H') || (c == 'h'))
    return HIGH;
  else
    return -1;
}

// ASCIItoInt
// Helper function to turn an ASCII hex value into a 0-15 byte val
int ASCIItoInt(char c)
{
  if ((c >= '0') && (c <= '9'))
    return c - 0x30; // Minus 0x30
  else if ((c >= 'A') && (c <= 'F'))
    return c - 0x37; // Minus 0x41 plus 0x0A
  else if ((c >= 'a') && (c <= 'f'))
    return c - 0x57; // Minus 0x61 plus 0x0A
  else
    return -1;
}

// printMenu
// A big ol' string of Serial prints that print a usage menu over
// to the other XBee.
void printMenu()
{
  // Everything is "F()"'d -- which stores the strings in flash.
  // That'll free up SRAM for more importanat stuff.
  XBee.println();
  XBee.println(F("Arduino XBee Remote Control!"));
  XBee.println(F("============================"));
  XBee.println(F("Usage: "));
  XBee.println(F("w#nnn - analog WRITE pin # to nnn"));
  XBee.println(F("  e.g. w6088 - write pin 6 to 88"));
  XBee.println(F("d#v   - digital WRITE pin # to v"));
  XBee.println(F("  e.g. ddh - Write pin 13 High"));
  XBee.println(F("r#    - digital READ digital pin #"));
  XBee.println(F("  e.g. r3 - Digital read pin 3"));
  XBee.println(F("a#    - analog READ analog pin #"));
  XBee.println(F("  e.g. a0 - Read analog pin 0"));
  XBee.println();
  XBee.println(F("- Use hex values for pins 10-13"));
  XBee.println(F("- Upper or lowercase works"));
  XBee.println(F("- Use 0, l, or L to write LOW"));
  XBee.println(F("- Use 1, h, or H to write HIGH"));
  XBee.println(F("============================"));  
  XBee.println();
}

