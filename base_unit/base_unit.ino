// SoftwareSerial is used to communicate with the XBee
#include <SoftwareSerial.h>

// LCD shield uses I2C to send and receive messages
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

// Motor driver requires TimerOne interrupt
#include <TimerOne.h>
#include <FrequencyTimer2.h>
#include <StepMotor.h>

#define SERIAL

#ifdef XBEE
  #define COM Xbee
#endif
#ifdef SERIAL
  #define COM Serial
#endif

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
const int STEP_PER_ROT = 400;
const int MOTOR_COUNT = 2;
const int RPM_INC = 3;                        // How many RPM each increase/decrease should increment

// Instantiate motor objects
StepMotor motor[2] = {StepMotor(STEP_PER_ROT, 6, 5, 8, 5, 4), StepMotor(STEP_PER_ROT, 11, 12, 9, 10, 13)};

void setup(){
  //******** XBee Setup ********//
  
  // Initialize XBee Software Serial port. Make sure the baud
  // rate matches your XBee setting (9600 is default).
  Serial.begin(9600); 

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
}

void loop(){
  // In loop() we continously check to see if a command has been received.
  checkInput();
  
  /* 
    Also check to see if either of the motor speeds have 
    changed and update the LCD to reflect that
  */
  for(int i = 0; i < MOTOR_COUNT; i++){
    if(motor[i].updateRequired()){
      updateLCD();
    }
  }
}

void checkInput(){
    if (COM.available())
  {
    char c = COM.read();
    switch (c)
    {
    case'c':
    case'C':
      changeMotorSelect();
      break;
    case 'u':      
    case 'U':      
      increaseSpeed();
      break;
    case 'd':      
    case 'D':
      decreaseSpeed(); 
      break;
    case 's':
    case 'S':
      setMotSpeed();
      break;
    case 'm':
    case 'M':
      setMicrosteps();
      break;
    }
  }
}

void updateLCD(){
  // Get the current motor's speed as RPM and cast as int
  int which = selectedMotor();
  int rpm = (int)motor[which].rpm();

  // Clear that motor's LCD line
  lcd.setCursor(6, which);
  lcd.print("         "); 

  /*
    If it's a positive value, add an extra character of padding. This
    will cause the numeric parts of negative and positive values to
    both start in the same character position.
  */
  if(rpm >= 0)
    lcd.setCursor(7, which);
  else
    lcd.setCursor(6, which);
  lcd.print(rpm); 
}


//******** COM Command Handlers ********//

void changeMotorSelect(){
  while (COM.available() < 1);               // Wait for motor and setting to be retrieved
  motorSelect = ASCIItoInt(COM.read()); 
  Serial.print("Selected motor ");
  Serial.println(motorSelect);
  updateLCD();
}

void increaseSpeed(){
  Serial.println("Increasing motor speed");
  int rpm = motor[selectedMotor()].rpm() + RPM_INC;
  motor[selectedMotor()].rpm(rpm);
  reportSpeed();
}

void decreaseSpeed(){
  Serial.println("Decreasing motor speed");
  int rpm = motor[selectedMotor()].rpm() - RPM_INC;
  motor[selectedMotor()].rpm(rpm);
  reportSpeed();
}

void setMicrosteps(){  
  while (COM.available() < 1);             // Wait for motor and setting to be retrieved
  int ms_level = ASCIItoInt(COM.read());   // Convert serial input to an into value
  /*
    Would rather have the Xbee radio send just a single character, so convert the
    five different levels into real microstep settings.
  */
  int new_ms = 1;
  switch(ms_level){
    case 1:
      new_ms = 1;
      break;
    case 2:
      new_ms = 2;
      break;
    case 3:
      new_ms = 4;
      break;
    case 4:
      new_ms = 8;
      break;
    case 5:
      new_ms = 16;
      break;
  }
  motor[selectedMotor].ms(new_ms);
  Serial.println("Setting " + String(selectedMotor()) + " microsteps:" + String(new_ms);
}

void setMotSpeed(){
  Serial.println("Setting motor speed");
  while (COM.available() < 4);             // Wait for pin and three value numbers to be received
  int dir = ASCIItoHL(COM.read());         // Read the direction
  int rpm = ASCIItoInt(COM.read()) * 100;  // Convert next three
  rpm += ASCIItoInt(COM.read()) * 10;      // chars to a 3-digit
  rpm += ASCIItoInt(COM.read());           // number.
  rpm = dir ? rpm : -rpm;                  // If direction is false, then the RPM value should be negative

  // Assign the new speed
  motor[selectedMotor()].rpm(rpm);

  // Print the new speed to the serial monitor
  reportSpeed();
}

// Prints the speed of the currently selected motor
void reportSpeed(){
  Serial.println("Motor " + String(selectedMotor()) + " speed: " + String((int)motor[selectedMotor()].rpm()));
}

//******** Helper Functions ********//

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
  /*XBee.println();
  XBee.println(F("Motor Driver XBee Remote Control!"));
  XBee.println(F("============================"));
  XBee.println(F("Connection verified!"));*/
}

/**
  Returns the number of the currently selected motor
*/
int selectedMotor(){
  for(int i = 0; i < MOTOR_COUNT; i++){
    if(motor[i].isSelected()){
      return i;
    }
  }
}