// SoftwareSerial is used to communicate with the XBee
#include <SoftwareSerial.h>

//******** XBee Vars ********//

SoftwareSerial XBee(11, 10); // Arduino RX, TX (XBee Dout, Din)

//******** Button Vars ********//

const int BUTTON_COUNT = 4;
const int BUTTON[BUTTON_COUNT] = {6, 8, 4, 5};
const int NULL_VAL = -1;
long startTime = NULL_VAL;
long pressedTime = NULL_VAL;
int curPress = NULL_VAL;
const int LONG_THRESH = 1000;
int motorSelect = 0;


void setup() {
  XBee.begin(9600);
  Serial.begin(9600);
  for(int i = 0; i < BUTTON_COUNT; i++){
    pinMode(BUTTON[i], INPUT);
  }
  Serial.println("Setup OK!");
}

void selectMotor(int button){
  switch(button){
    case 0:
    case 1:
      XBee.print("a");
      Serial.println("Selecting motor 0");
      break;
    case 2:
    case 3:
      XBee.print("b");
      Serial.println("Selecting motor 1");
      break;
  }
}

void adjustSpeed(int button){
      switch(button){
        case 0:
        case 3:
          XBee.print('u');
          Serial.println("Increasing speed");
          break;
        case 1:
        case 2:
          XBee.print('d');
          Serial.println("Decreasing speed");
          break;
        default:
          break;
    }
}

void adjustSpeedRapid(int button){
      switch(button){
        case 0:
        case 3:
          XBee.print('U');
          Serial.println("Increasing speed rapidly");
          break;
        case 1:
        case 2:
          XBee.print('D');
          Serial.println("Decreasing speed rapidly");
          break;
        default:
          break;
    }
}

void loop() {

  delay(250);
  
  // Check for new button presses
  if(curPress == NULL_VAL){
    for(int i = 0; i < BUTTON_COUNT; i++){
      if(digitalRead(BUTTON[i]) == HIGH){
        Serial.println("Button " + String(i) + " detected");
        curPress = i;
        startTime = millis();
        selectMotor(curPress);
        break;
      }
    }
  }
  // Check if button is still pressed or has been released
  else{
    // Continuing press
    boolean pressed = digitalRead(BUTTON[curPress]);
    if(pressed){
      pressedTime = millis() - startTime;
      // If held sufficiently long, execute the fast adjust action
      if(pressedTime > LONG_THRESH){
        adjustSpeedRapid(curPress);
      }
    }
    // Released press
    else{
      // If this was a short press, execute the slow action
      if(pressedTime < LONG_THRESH){
        adjustSpeed(curPress);
      }
      pressedTime = NULL_VAL;
      curPress = NULL_VAL;
    }
  }
}



