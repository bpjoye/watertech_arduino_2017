void setup() {

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);

}

int bypass = 0;

void loop() {

  if (int(Serial.read()) == 10) {

    if (digitalRead(LED_BUILTIN) == LOW) {
      digitalWrite(LED_BUILTIN, HIGH);
      bypass = 1;
    }
      
    if (digitalRead(LED_BUILTIN) == HIGH && bypass == 0) {
      digitalWrite(LED_BUILTIN, LOW);
    }

    bypass = 0;
  }
}
