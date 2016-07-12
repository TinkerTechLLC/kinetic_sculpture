//******** Motor Control ********//

// Interrupt function for motor 0
void stepMotor0(){
  PORTD |= on[0];
  delayMicroseconds(1);
  PORTD &= off[0];
}

// Interrupt function for motor 1
void stepMotor1(){
  PORTB |= on[1];
  delayMicroseconds(1);
  PORTB &= off[1];
}

// Sets the interrupt timing for new motor speeds
void updateTiming(){
  boolean noUpdate[2] = {false, false};
  updateTiming(noUpdate);
}

void updateTiming(boolean* msUpdate){
    for(int i = 0; i < MOTOR_COUNT; i++){
      // Only update motors that have changed speeds
      if(targetSpd[i] == curSpd[i] && !msUpdate[i]){
        continue;
      }
        
      // Disable the interrupt of a motor with 0 speed
      if(targetSpd[i] == 0){
        if(i == 0){
          Timer1.detachInterrupt(); 
       }
        else if(i == 1){
          FrequencyTimer2::setOnOverflow(0); 
        }
        continue;
      }
      // Otherwise, determine the new delay interval...
      else{
        float msMultiplier = ms[i] == HIGH ? 4 : 1;
        float stepsPerMin = (float)targetSpd[i] * (float)STEP_PER_ROT * msMultiplier;
        float stepsPerSec = stepsPerMin / SEC_PER_MIN;
        COM.print("Motor " + String(i) + " steps per min: " + String(stepsPerMin) +
          " Steps per sec: " + String(stepsPerSec));
        stepDelay[i] = round((float)MICROS_PER_SEC / stepsPerSec);
      } 
      COM.print(" Current RPM: ");
      COM.print(targetSpd[i]);
      COM.print(" Current delay: ");
      COM.println(stepDelay[i]);

      curSpd[i] = targetSpd[i];
      // ... and assign it to the proper interrupt timer
      if(i == 0){
        COM.println("Updating motor 0 interrupt");
        Timer1.attachInterrupt(stepMotor0, stepDelay[0] >= 0 ? stepDelay[0] : -stepDelay[0]);
      }
      else if(i == 1){
        COM.println("Updating motor 1 interrupt");
        FrequencyTimer2::setOnOverflow(stepMotor1);
        FrequencyTimer2::setPeriod(stepDelay[1] >= 0 ? stepDelay[1]*2 : -stepDelay[1]*2);
      }
    }
}

// Sets the DIR pins
void updateDirPins(){
  for(int i = 0; i < MOTOR_COUNT; i++){
    digitalWrite(DIR[i], getDir(i));
  }
}

// Returns the direction based upon current target speed
int getDir(int motor){
  if(targetSpd[motor] >= 0){
    return 1;
  }
  else{
    return 0;
  }
}

//******** COM Command Handlers ********//

void changeMotorSelect(){
  while (COM.available() < 1);               // Wait for motor and setting to be retrieved
  motorSelect = ASCIItoInt(COM.read());  
  updateLCD();
}

void increaseSpeed(){
  COM.println("Increasing motor speed");
  targetSpd[motorSelect] += RPM_INC;
  reportSpeed(motorSelect);
  updateRequired = true;
}

void decreaseSpeed(){
  COM.println("Decreasing motor speed");
  targetSpd[motorSelect] -= RPM_INC;
  reportSpeed(motorSelect);
  updateRequired = true;
}

void setMotSpeed(){
  COM.println("Setting motor speed");
  while (COM.available() < 4);               // Wait for pin and three value numbers to be received
  int newDir = ASCIItoHL(COM.read());        // Read the direction
  int value = ASCIItoInt(COM.read()) * 100;  // Convert next three
  value += ASCIItoInt(COM.read()) * 10;      // chars to a 3-digit
  value += ASCIItoInt(COM.read());           // number.

  // Set the motor values
  targetSpd[motorSelect] = newDir == 1 ? value : -value;
  reportSpeed(motorSelect);
  updateRequired = true;
}

void setMicrosteps(){
  while (COM.available() < 1);               // Wait for motor and setting to be retrieved
  ms[motorSelect] = ASCIItoInt(COM.read());  // Convert to an int value and save to microstep array
  updateRequired = true;
  COM.println("Setting " + String(motorSelect) + " microsteps:" + String(ms[motorSelect]));
  updateMsPins();
}

void updateMsPins(){
  static int lastMs[2] = {DEFAULT_MS, DEFAULT_MS};
  boolean updateMs[2] = {false, false};
  if(lastMs[motorSelect] != ms[motorSelect]){
    switch(ms[motorSelect]){
    case 1:
      digitalWrite(MS1[motorSelect], LOW);
      digitalWrite(MS2[motorSelect], LOW);
      digitalWrite(MS3[motorSelect], LOW);
      break;
    case 2:
      digitalWrite(MS1[motorSelect], HIGH);
      digitalWrite(MS2[motorSelect], LOW);
      digitalWrite(MS3[motorSelect], LOW);
      break;
    case 4:
      digitalWrite(MS1[motorSelect], LOW);
      digitalWrite(MS2[motorSelect], HIGH);
      digitalWrite(MS3[motorSelect], LOW);
      break;
    case 8:
      digitalWrite(MS1[motorSelect], HIGH);
      digitalWrite(MS2[motorSelect], HIGH);
      digitalWrite(MS3[motorSelect], LOW);
      break;
    case 16:
      digitalWrite(MS1[motorSelect], HIGH);
      digitalWrite(MS2[motorSelect], HIGH);
      digitalWrite(MS3[motorSelect], HIGH);
      break; 
    }
    // Indicator to pass timing updater to account for
    // change in microsteps
    updateMs[motorSelect] = true;
  }
  updateTiming(updateMs);
}

void reportSpeed(int motor){
  COM.println("Motor " + String(motor) + " speed: " + targetSpd[motor]);
}

