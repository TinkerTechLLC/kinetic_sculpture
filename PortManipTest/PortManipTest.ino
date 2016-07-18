const int DELAY(1000);

void setup() {
  Serial.begin(9600);
  
  for(int i = 0; i < 13; i++){
    pinMode(i, OUTPUT);
  }
}

void loop() {
  /*
  Serial.println("0");
  PORTD = B00000001;
  delay(DELAY);
  Serial.println("1");
    PORTD = B00000010;
  delay(DELAY);
  Serial.println("2");
    PORTD = B00000100;
  delay(DELAY);
  Serial.println("3");
    PORTD = B00001000;
  delay(DELAY);
  Serial.println("4");
    PORTD = B00010000;
  delay(DELAY);
  Serial.println("5");
    PORTD = B00100000;
  delay(DELAY);
  Serial.println("6");
    PORTD = B01000000;
  delay(DELAY);
  Serial.println("7");
    PORTD = B10000000;
  delay(DELAY);
  */
  Serial.println("8");
    PORTD = B00000000;
    PORTB = B00000001;
  delay(DELAY);
    Serial.println("9");
    PORTB = B00000010;
  delay(DELAY);
      Serial.println("10");
    PORTB = B00000100;
  delay(DELAY);
      Serial.println("11");
    PORTB = B00001000;
  delay(DELAY);
      Serial.println("12");
    PORTB = B00010000;
  delay(DELAY);
      Serial.println("13");
    PORTB = B00100000;
  delay(DELAY);


}
