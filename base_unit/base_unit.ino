// SoftwareSerial is used to communicate with the XBee
#include <SoftwareSerial.h>

// LCD shield uses I2C to send and receive messages
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

// Motor driver requires TimerOne interrupt
#include <TimerOne.h>
#include <FrequencyTimer2.h>


//******** XBee Vars ********//

SoftwareSerial XBee(2, 3); // Arduino RX, TX (XBee Dout, Din)

//******** LCD Vars ********/

// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// These #defines make it easy to set the backlight color
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

//******** Motor Vars ********//

// Constant reference values
const long MICROS_PER_SEC = 1000000;
const int SEC_PER_MIN = 60;
const int STEP_PER_ROT = 200;
const int MOTOR_COUNT = 2;
const int RPM_INC = 3;                        // How many RPM each increase/decrease should increment

// Pin assignments
const int DIR[2] = {5, 11};
const int STEP[2] = {6, 12};
const int MS1[2] = {8, 9};
const int MS2[2] = {5, 10};
const int MS3[2] = {4, 13};

// Motor values
int targetSpd[2] = {0, 0};                    // The targer motor speed. Default to 0 RPM
int curSpd[2] = {0, 0};                       // The actual current motor speed. Will not equal target speed while performing accelerations.
int ms[2] = {HIGH, HIGH};                     // Default to quarter-stepping (HIGH). LOW indicates full-stepping.
long stepDelay[2];                            // Microsecond delay between steps
long lastStepTime[2] = {0, 0};                // Time of last step in microseconds
boolean updateRequired = false;               // Flag to indicate timing and LCD update

const byte on[2] = {B01000000, B00000100};    // High comparison states for switching step pins
const byte off[2] = {B10111111, B11111011};   // Low comparison states for switching step pins
int motorSelect = 0;

void setup()
{
  //******** XBee Setup ********//
  
  // Initialize XBee Software Serial port. Make sure the baud
  // rate matches your XBee setting (9600 is default).
  XBee.begin(9600); 
  printMenu(); // Print a helpful menu:


  //******** LCD Setup ********//

  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.setBacklight(BLUE);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Above: 0");
  lcd.setCursor(0, 1);
  lcd.print("Below: 0");

  //******** Motor Setup ********//
  Serial.begin(9600);
  for(int i = 0; i < MOTOR_COUNT; i++){
    pinMode(DIR[i], OUTPUT);
    pinMode(STEP[i], OUTPUT);
    pinMode(MS[i], OUTPUT);
  }
  updateMsPins();
  updateDirPins();

  // Setup the timers
  Timer1.initialize();                        // Get Timer1 ready to accept function and timing
  FrequencyTimer2::disable();                 // This disables toggling of pin 11 at every interrupt
  FrequencyTimer2::setOnOverflow(0);          // Initially set the interrupt function to null
}

void loop()
{
  // In loop() we continously check to see if a command has been received.
  checkXBee();

  if(updateRequired){
    updateLCD();
    updateMsPins();
    updateTiming();
    updateDirPins();
    updateRequired = false;
  }
}

void checkXBee(){
    if (XBee.available())
  {
    char c = XBee.read();
    switch (c)
    {
    case'c':
    case'C':
      changeMotorSelect();
      break;
    case 'u':      // If received 'w'
    case 'U':      // or 'W'
      increaseSpeed(); // Increase specified motor speed by 3 rpm
      break;
    case 'd':      // If received 'd'
    case 'D':      // or 'D'
      decreaseSpeed(); // Decrease specified motor speed by 3 rpm
      break;
    case 's':      // If received 'r'
    case 'S':      // or 'R'
      setMotSpeed();  // Set speed of specified motor
      break;
    case 'm':
    case 'M':
      setMicrosteps(); // Set the microstep setting of the specified motor
      break;
    }
  }
}

void updateLCD(){
  static int oldSpd[2] = {0, 0};
  
  for(int i = 0; i < MOTOR_COUNT; i++){
    if(oldSpd[i] != targetSpd[i]){
      lcd.setCursor(7, i);
      lcd.print("         "); 
      lcd.setCursor(7, i);
      lcd.print(targetSpd[i]); 
      oldSpd[i] = targetSpd[i];
    }
  }
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

//******** Helper Functions ********//

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
  /*XBee.println();
  XBee.println(F("Motor Driver XBee Remote Control!"));
  XBee.println(F("============================"));
  XBee.println(F("Connection verified!"));*/
}


