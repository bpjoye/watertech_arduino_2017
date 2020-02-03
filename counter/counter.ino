void setup() {

  pinMode(6, OUTPUT);
  pinMode(11, INPUT);

  digitalWrite(6, HIGH);

  Serial.begin(9600);
}

int x = 0;
bool readyToLoop = true;

void loop() {

  if (readyToLoop == true && digitalRead(11) == HIGH) {
    
    x++;
    Serial.print(x);
    Serial.print(",");

    readyToLoop = false; 
  }

  if (readyToLoop == false && digitalRead(11) == LOW) {

    readyToLoop = true;
  }

} 
