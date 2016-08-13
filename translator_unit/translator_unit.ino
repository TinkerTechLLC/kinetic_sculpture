/*
 * This is the firmware that should be loaded onto the
 * Adafruit Trinket Pro 5V MCU that will act as the
 * XBee to hardware serial translator unit. Its only function
 * is to receive the XBee radio input and pass it along
 * to the main MCU's serial pins.
 * 
 * This is necessary because when the main MCU was trying
 * to handle incoming XBee radio communication, it caused
 * hiccups in the motor movement, presumably because it
 * used a soft serial input instead of the hardware serial
 * input.
 * 
 * Author: Michael Ploof
 * Date: 8/10/16
 */


// SoftwareSerial is used to communicate with the XBee
#include <SoftwareSerial.h>
SoftwareSerial XBee(4, 5); // Arduino RX, TX (XBee Dout, Din)

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  XBee.begin(9600);
}

// the loop function runs over and over again forever
void loop() {
  checkInput();
}

void checkInput(){
  if(XBee.available()){
    char c = XBee.read();
    Serial.println(c);
  }
}

