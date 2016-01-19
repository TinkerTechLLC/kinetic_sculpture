//******** Motor Control ********//

void stepMotor(){
  PORTD |= on[0];
  PORTB |= on[1];
  delayMicroseconds(1);
  PORTD &= off[0];
  PORTB &= off[1];
}

void updateTiming(){
  
}

void updateDirPins(){
  for(int i = 0; i < MOTOR_COUNT; i++){
    digitalWrite(DIR[i], getDir(i));
  }
}

void updateMsPins(){
  for(int i = 0; i < MOTOR_COUNT; i++){
    digitalWrite(MS[i], ms[i]);
  }
}

int getDir(int motor){
  if(targetSpd[motor] >= 0){
    return 1;
  }
  else{
    return 0;
  }
}

//******** XBee Command Handlers ********//

void increaseSpeed(){
  Serial.println("Increasing motor speed");
  while (XBee.available() < 1);               // Wait for motor to become available
  int motor = ASCIItoInt(XBee.read());
  targetSpd[motor] += RPM_INC;
  reportSpeed(motor);
  updateRequired = true;
}

void decreaseSpeed(){
  Serial.println("Decreasing motor speed");
  while (XBee.available() < 1);               // Wait for motor to become available
  int motor = ASCIItoInt(XBee.read());
  targetSpd[motor] -= RPM_INC;
  reportSpeed(motor);
  updateRequired = true;
}

void setMotSpeed(){
  Serial.println("Setting motor speed");
  while (XBee.available() < 4);               // Wait for pin and three value numbers to be received
  int motor = ASCIItoInt(XBee.read());        // Read in the motor number
  int newDir = ASCIItoHL(XBee.read());        // Read the direction
  int value = ASCIItoInt(XBee.read()) * 100;  // Convert next three
  value += ASCIItoInt(XBee.read()) * 10;      // chars to a 3-digit
  value += ASCIItoInt(XBee.read());           // number.
  value = constrain(value, 0, 350);           // Constrain that number.

  // Set the motor values
  targetSpd[motor] = newDir == 1 ? value : -value;
  reportSpeed(motor);
  updateRequired = true;
}

void flipMotor(){
  Serial.println("Flipping motor direction");
  while (XBee.available() < 1);               // Wait for motor to become available
  char motor = ASCIItoInt(XBee.read());
  targetSpd[motor] = -targetSpd[motor];
  reportSpeed(motor);
  updateRequired = true;
}

void setMicrosteps(){
  Serial.println("Setting microsteps");
  while (XBee.available() < 2);               // Wait for motor and setting to be retrieved
  int motor = ASCIItoInt(XBee.read());        // Read in the motor number
  ms[motor] = ASCIItoHL(XBee.read());         // Convert to a HIGH / LOW value and save to microstep array
  updateRequired = true;
}

void reportSpeed(int motor){
  Serial.println("Motor " + String(motor) + " new speed: " + String(targetSpd[motor]));
}

