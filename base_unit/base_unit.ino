// SoftwareSerial is used to communicate with the XBee
#include <SoftwareSerial.h>

// LCD shield uses I2C to send and receive messages
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

// Motor driver requires TimerOne and FrequencyTimer libraries
#include <TimerOne.h>
#include <FrequencyTimer2.h>
#include <StepMotor.h>

#define XBEE              // Sets whether to run in Xbee or Serial mode. Change this to "SERIAL" to control via the unit via the serial monitor

#ifdef XBEE
  #define COM XBee
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
const int STEP_PER_ROT = 400;                 // Number of physical steps per motor. 400 for 0.9deg motors, 200 for 1.8deg motors.
const int MOTOR_COUNT = 2;                    // Total number of motors
const int RPM_INC = 3;                        // How many RPM each increase/decrease should increment
const int MAX_RPM = 348;                      // The maximum allowable RPM for either motor

// Motor properties
int sel_mot = 0;                              // The currently selected motor
int target_rpm[MOTOR_COUNT] = {30, 30};       // Change the values in the brackets to set the speed to which the motors will initially ramp
bool flipped[MOTOR_COUNT] = {true, false};    // Whether the motor directions should be flipped from their normal state. Having one flipped
                                              // will cause them to both have the same positive direction when facing each other
StepMotor motor[MOTOR_COUNT];                 // The motor objects, initialized in setup()

/*
  FYI, motor[0] is "Above" and motor[1] is "Below"
*/

void setup(){
  //******** Motor Setup ********//
  
  // Initialize the motor objects
  for(int i = 0; i < MOTOR_COUNT; i++){
    motor[i] = StepMotor(STEP_PER_ROT);
    motor[i].rpm(0);
    motor[i].flip(flipped[i]);
  }
  
  //******** XBee / Serial Setup ********//
  
  // Initialize XBee Software Serial port. Make sure the baud
  // rate matches your XBee setting (9600 is default).
  Serial.begin(9600); 
  XBee.begin(9600);


  //******** LCD Setup ********//

  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.setBacklight(BLUE);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Above: 0");
  lcd.setCursor(0, 1);
  lcd.print("Below: 0");
  updateLCD();
}

void loop(){
  
  updateRamping();
  
  // In loop() we continously check to see if a command has been received and handle it
  checkInput();
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
    }
  }
}

void updateLCD(){
  for(int i = 0; i < MOTOR_COUNT; i++){
    // Clear that motor's LCD line
    lcd.setCursor(6, i);
    lcd.print("         "); 
    
    /*
      If it's a positive value, add an extra character of padding. This
      will cause the numeric parts of negative and positive values to
      both start in the same character position.
    */
    if(target_rpm[i] >= 0)
      lcd.setCursor(7, i);
    else
      lcd.setCursor(6, i);
    lcd.print(target_rpm[i]); 
  }
}


//******** COM Command Handlers ********//

void changeMotorSelect(){
  while (COM.available() < 1);               // Wait for motor and setting to be retrieved
  sel_mot = ASCIItoInt(COM.read()); 
  Serial.print("Selected motor ");
  Serial.println(sel_mot);
  updateLCD();
}

void increaseSpeed(){
  Serial.println("Increasing target speed");
  target_rpm[sel_mot] += RPM_INC;
  if(target_rpm[sel_mot] > MAX_RPM)
    target_rpm[sel_mot] = MAX_RPM;
  reportSpeed();
  updateLCD();
}

void decreaseSpeed(){
  Serial.println("Decreasing target speed");
  target_rpm[sel_mot] -= RPM_INC;
  if(target_rpm[sel_mot] < -MAX_RPM)
    target_rpm[sel_mot] = -MAX_RPM;
  reportSpeed();
  updateLCD();
}

// Prints the speed of the currently selected motor
void reportSpeed(){
  Serial.println("Motor " + String(sel_mot) + " target speed: " + String((int)target_rpm[sel_mot]) 
    + " motor speed: " + String((int)motor[sel_mot].rpm()));
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

void updateRamping(){
  static int last_update = millis();  // Time stamp of last update
  const float RAMP_INC = 0.5;         // By how much the speed should be adjusted each increment
  const int UPDATE_TIME = 45;         // How often the speeds should be updated
  const float THRESHOLD = 0.4;        // How close is "good enough"
  
  // If we've waited long enough, update the speeds for each motor
  if(millis() - last_update > UPDATE_TIME){
    for(int i = 0; i < MOTOR_COUNT; i++){
      float new_spd; 
      // If the target speed is higher than current, increase the speed...
      if(target_rpm[i] - motor[i].rpm() > THRESHOLD){
        new_spd = motor[i].rpm() + RAMP_INC;
        motor[i].rpm(new_spd);
      }
      // ...otherwise decrease it
      else if(target_rpm[i] - motor[i].rpm() < -THRESHOLD){
        new_spd = motor[i].rpm() - RAMP_INC;
        motor[i].rpm(new_spd);
      }
      else{
        // If we're within the threshold, don't adjust the speed.
      }
    }
    last_update = millis();
  }
}
