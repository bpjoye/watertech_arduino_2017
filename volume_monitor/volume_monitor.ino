void setup() {
  cli();

  Serial.begin(9600);
  pinMode(A0, INPUT);

  //set timer1 interrupt at 0.5Hz
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; //initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 31249; // (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

   
  sei();
} 

int timerLimit = 0; //controls interrupt frequency (measured in twos)
int switchCounter = 0; //number of times the switch is toggled
int arrayInt = 0; // 
bool isReadyState = true; //makes sure it doesn't read more than once
bool doneBuffering = false; //enables a buffer before it begins data collection
int dataArray[7]; //Array to hold information as it comes in
float runningTotal = 0; //
float volumeScaler = 0.14511; //Change this value depending on the pipe measurements

void loop() {

  if (digitalRead(A0) == HIGH && isReadyState == true) {
    switchCounter++;
    isReadyState = false;
  }
  
  if (digitalRead(A0) == LOW) { //allows another count once the switch is toggled back
    isReadyState = true;
  }
} 

ISR(TIMER1_COMPA_vect) { 

  timerLimit++;

  if (timerLimit == 5) { //Put interrupt code inside here

    if (doneBuffering == false) {

      dataArray[arrayInt] = switchCounter;   
      Serial.println(switchCounter);
      arrayInt++;

      if (arrayInt == 6) {
        dataArray[arrayInt] = 0;
        doneBuffering = true;
        //Serial.println();
        //Serial.println("Data Collection Beginning, Volume per minute (60 second window average):");
      } 
    }
    
    if (doneBuffering == true) {

      dataArray[6] = switchCounter;

      for (int i=0; i<6; i++) { //Move all values down one spot (deletes oldest value)

        dataArray[i] = dataArray[i+1];
      }
      for (int i=0; i<6; i++) { //Sum the six values in dataArray

        runningTotal += dataArray[i];
      }

      Serial.print(switchCounter); Serial.print(", past 60 seconds: "); Serial.print(runningTotal); Serial.println();
      runningTotal = 0;
    }   

    switchCounter = 0;
    timerLimit = 0;
  }
}







