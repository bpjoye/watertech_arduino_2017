void setup() {

  Serial.begin(9600);
}

int x = 0;

void loop() {

Serial.print(byte(x));
Serial.print(48);

x++;
}
