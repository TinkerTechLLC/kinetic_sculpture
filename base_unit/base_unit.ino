/*
 *  This is the firmware that should be loaded onto the base Arduino Uno MCU. 
 *  
 *  In order for this to compile properly, you must also have the "Step Motor" library
 *  installed. To install the library, go to https://github.com/TinkerTechLLC/TT_StepperMotorShield_Library
 *  and download the zip file of the repository. Create a new directory called 
 *  "StepperMotor" in your Arduino libraries directory. Unpack the zipped file and place 
 *  StepperMotor.cpp and StepperMotor.h inside the directory you just created.
 *  
 *  Author: Michael Ploof
 *  Date: 8/13/16
 */

// LCD shield uses I2C to send and receive messages
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

// Motor driver requires TimerOne and FrequencyTimer libraries
#include <TimerOne.h>
#include <FrequencyTimer2.h>
#include <StepMotor.h>

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
const int RPM_INC_RAPID = RPM_INC * 5;        // How many RPM each rapid increase/decrease should increment
const int MAX_RPM = 348;                      // The maximum allowable RPM for either motor

// Motor properties
int sel_mot = 0;                              // The currently selected motor
int target_rpm[MOTOR_COUNT] = {30, 30};       // Change the values in the brackets to set the speed to which the motors will initially ramp
bool flipped[MOTOR_COUNT] = {true, false};    // Whether the motor directions should be flipped from their normal state. Having one flipped
                                              // will cause them to both have the same positive direction when facing each other
StepMotor motor[MOTOR_COUNT];                 // The motor objects, initialized in setup()
bool update_lcd = true;

/*
  FYI, motor[0] is "Above" and motor[1] is "Below"
*/

void setup(){
    
  //******** Serial Setup ********//

  Serial.begin(9600);
  
  //******** Motor Setup ********//
  
  // Initialize the motor objects
  for(int i = 0; i < MOTOR_COUNT; i++){
    motor[i] = StepMotor(STEP_PER_ROT);
    motor[i].rpm(0);
    motor[i].flip(flipped[i]);
  }
  
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

/*
 * This is the main program loop. After executing
 * the setup() function, the MCU will continuously
 * loop the contents of this function.
 */
void loop(){

  // Increase or decrease the motor speeds as necessary
  updateRamping();
  
  // In loop() we continously check to see if a command has been received and handle it
  checkInput();

  // Update the LCD. The updateLCD() function will only redraw the LCD screen if necessary
  updateLCD();
}

//******** Update Functions ********//

/**
 * This function checks whether there is any data
 * on the serial buffer and if there is, will
 * execute the appropriate command.
 */
void checkInput(){
  // If there's a command in the serial buffer, execute
  // the appropriate corresponding command
  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
    case'a':
    case'A':
      changeMotorSelect(0);
      break;
    case'b':
    case'B':
      changeMotorSelect(1);
      break;
    case 'u':     
      increaseSpeed(RPM_INC);
      break; 
    case 'U':      
      increaseSpeed(RPM_INC_RAPID);
      break;
    case 'd':    
      decreaseSpeed(RPM_INC); 
      break;  
    case 'D':
      decreaseSpeed(RPM_INC_RAPID); 
      break;
    }
  }
}

/*
 * This function checks whether the LCD needs to be updated
 * and prints the new motor speed values if necessary.
 */
void updateLCD(){
  static long last_update = 0;
  const int UPDATE_THRESHOLD = 100;

  // Only update if there's been enough time since the last
  // update and if an update has been requested.
  if(millis() - last_update > UPDATE_THRESHOLD && update_lcd){
    // Update both motor lines
    for(int i = 0; i < MOTOR_COUNT; i++){
      // Set the LCD's cursor position
      lcd.setCursor(6, i);

      /*
       * This is VERY important. I had previously been commanding
       * the LCD to print a row of blank spaces, then printing the
       * new speed over top of that. For some reason, this caused
       * either one or both motors to not move. I think it was because
       * the Wire library that the LCD library depends upon also uses
       * interrupts and by having more characters to write, was somehow
       * causing interrupt conflicts.
       * 
       * So: DO NOT CHANGE THIS FUNCTION
       */
      // Construct the new LCD text
      String out = String(target_rpm[i]);
      if(target_rpm >= 0){
        out = " " + out;
      }
      if(abs(target_rpm[i]) < 100){
        out = out + " ";
      }

      // Print the new text to the LCD
      lcd.print(out); 
    }
    // Turn off the update request flag
    update_lcd = false;
  }
}

/**
 * This function checks whether each of the current motor
 * speeds is within a small threshold of the target speed,
 * and if it is not, will ramp the motor speed until it
 * achieves the target speed.
 */
void updateRamping(){
  static long last_update = millis();  // Time stamp of last update
  const float RAMP_INC = 1;           // By how much the speed should be adjusted each increment
  const int UPDATE_TIME = 20;         // How often the speeds should be updated
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

//******** COM Command Handlers ********//

/*
 *  Sets the currently selected motor. 
 */
void changeMotorSelect(int which){
 sel_mot = which;
 update_lcd = true;
}

/*
 * Increases the target speed of the selected
 * motor by the specified increment.
 */
void increaseSpeed(int p_increment){
  target_rpm[sel_mot] += p_increment;
  if(target_rpm[sel_mot] > MAX_RPM)
    target_rpm[sel_mot] = MAX_RPM;
  update_lcd = true;
}

/*
 * Decreases the target speed of the selected
 * motor by the specified increment.
 */
void decreaseSpeed(int p_increment){
  target_rpm[sel_mot] -= p_increment;
  if(target_rpm[sel_mot] < -MAX_RPM)
    target_rpm[sel_mot] = -MAX_RPM;
  update_lcd = true;
}
