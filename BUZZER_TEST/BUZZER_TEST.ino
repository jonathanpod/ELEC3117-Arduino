void setup() {
  // Buzzer is GPIO15
  
  // Initialize Serial Monitor
  Serial.begin(9600);
  pinMode(15, OUTPUT);
  
}

void loop() { 
  digitalWrite(15, HIGH);
  delay(2);
  digitalWrite(15, LOW);
  delay(2);
}
