#include <SoftwareSerial.h>         //Include the software serial library  

SoftwareSerial sSerial(11, 10);     // RX, TX  - Name the software serial library sftSerial (this cannot be omitted)
                                    // assigned to pins 10 and 11 for maximum compatibility
int s0 = 7;                         // Tentacle uses pin 7 for multiplexer control S0
int s1 = 6;                         // Tentacle uses pin 6 for multiplexer control S1
int enable_1 = 5;                   // Tentacle uses pin 5 to control pin E on shield 1

char computerdata[20];              // A 20 byte character array to hold incoming data from a pc/mac/other
char sensordata[30];                // A 30 byte character array to hold incoming data from the sensors
byte computer_bytes_received = 0;   // We need to know how many characters bytes have been received
byte sensor_bytes_received = 0;     // We need to know how many characters bytes have been received

char *channel;                      // Char pointer used in string parsing
char *cmd;                          // Char pointer used in string parsing




void setup() {
  
  cli();

  Serial.begin(9600);
  sSerial.begin(38400);
  

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

  welcome();
}

void loop() {

  if (computer_bytes_received != 0) {             // If computer_bytes_received does not equal zero
    channel = strtok(computerdata, ":");          // Let's parse the string at each colon
    cmd = strtok(NULL, ":");
    open_channel();                               // Call the function "open_channel" to open the correct data path
    if (cmd != 0) {                               // if no command has been sent, send nothing
      sSerial.print(cmd);                         // Send the command from the computer to the Atlas Scientific device using the softserial port
      sSerial.print("\r");                        // <CR> carriage return to terminate message
    }
    computer_bytes_received = 0;                  // Reset the var computer_bytes_received
  }

  if (sSerial.available() > 0) {                 // If data has been transmitted from an Atlas Scientific device
    sensor_bytes_received = sSerial.readBytesUntil(13, sensordata, 30); //we read the data sent from the Atlas Scientific device until we see a <CR>. We also count how many character have been received
    sensordata[sensor_bytes_received] = 0;       // we add a 0 to the spot in the array just after the last character we received. This will stop us from transmitting incorrect data that may have been left in the buffer
    Serial.println(sensordata);                  // let’s transmit the data received from the Atlas Scientific device to the serial monitor
  }
}

void open_channel() {

  switch (*channel) {

    case '0':                                // if channel==0 then we open channel 0
      digitalWrite(enable_1, LOW);           // setting enable_1 to low activates primary channels: 0,1,2,3
      digitalWrite(s0, LOW);                 // S0 and S1 control what channel opens
      digitalWrite(s1, LOW);                 // S0 and S1 control what channel opens
      break;

    case '1':
      digitalWrite(enable_1, LOW);
      digitalWrite(s0, HIGH);
      digitalWrite(s1, LOW);
      break;

    case '2':
      digitalWrite(enable_1, LOW);
      digitalWrite(s0, LOW);
      digitalWrite(s1, HIGH);
      break;

    case '3':
      digitalWrite(enable_1, LOW);
      digitalWrite(s0, HIGH);
      digitalWrite(s1, HIGH);
      break;
  }
}

void serialEvent() {               //This interrupt will trigger when the data coming from the serial monitor(pc/mac/other) is received
  computer_bytes_received = Serial.readBytesUntil(13, computerdata, 20); //We read the data sent from the serial monitor(pc/mac/other) until we see a <CR>. We also count how many characters have been received
  computerdata[computer_bytes_received] = 0;      //We add a 0 to the spot in the array just after the last character we received.. This will stop us from transmitting incorrect data that may have been left in the buffer
}

void welcome() {

  Serial.flush();
  Serial.println();
  Serial.print("READY!_");
}

ISR(TIMER1_COMPA_vect) {

  
}

