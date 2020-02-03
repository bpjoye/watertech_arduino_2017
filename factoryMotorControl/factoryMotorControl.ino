void setup() {
  
  pinMode(2, OUTPUT);

  Serial.begin(9600);
}

float frequency;
float delayTime = 0;
float input;
bool began = false;

void loop() {
  if (began) {
    if (frequency > 500) {
      digitalWrite(2, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(2, LOW);
      delayMicroseconds(delayTime);
    }
  }
}

void serialEvent() {

  while (Serial.available()) {
    began = false;
    input = Serial.readString().toInt();
    frequency = (input*20) + 500;

    if (frequency > 500)
      delayTime = ((1/frequency)/2)*1000000;
  }
  began = true;
}
