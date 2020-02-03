//  ******************************************** Cart 1/11/2016 ********************************************
/*
  SW tasks
     Fill pump shutoff when fluid container empty (>1 minute attempt to fill reservior)
*/

//  ******************************************** DEFINE VARIABLES ********************************************

// ADJUSTABLE TIMING VARIABLES
long splashScreenSeconds = 5; // Splash screen time (1 second minimum)
long roomClearSeconds = 5; // SECONDS TO CLEAR THE ROOM
long defaultRunTimeSeconds = 1800 ; // INITIAL DISINFECTING TIME IN SECONDS
long reserviorDrainTime = 115; // Seconds to drain the reservior (reduces left over fluid)


// TIMING VARIABLES
long runTimeSeconds = defaultRunTimeSeconds;
int remainingMinutes;
int remainingSeconds;
long startTime;

// DISPLAY SETUP
// SainsSmart 4x20 LCD serial display - requires I2C library
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal.h>
#define I2C_ADDR    0x27 // <<Display address found from I2C Scanner (note: other serial display drivers may be other addresses)
#define Rs_pin  0
#define Rw_pin  1
#define En_pin  2
#define BACKLIGHT_PIN     3
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7
LiquidCrystal_I2C  lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);

// DUAL MOTOR DRIVER SETUP
/*
  Required connections between Arduino and qik 2s9v1:
      Arduino   qik 2s9v1
  -------------------------
           5V - VCC
          GND - GND
  Digital Pin 11 - TX
  Digital Pin 12 - RX
  Digital Pin 13 - RESET
*/
#include <SoftwareSerial.h>
#include <PololuQik.h>
//#include <Arduino.h>
PololuQik2s9v1 qik(11, 12, 13);

// INPUT SETUP
#define liquidLevelUpper 50 // Input pin pulled LOW when fluid LOW

// OUTPUT SETUP
//#define VFDStop     46
//#define VFDStart    48
#define airPump1    49      // power PCB pin 1 - Active high
#define airPump2    51      // power PCB pin 3 - Active high
#define largeFan    53      // power PCB pin 5 - Active high
#define buzzer      34      // Active high 
#define buzzerGnd   40      // buzzer Gnd, keep LOW

// SWITCH SETUP                            **White Wires**
int switchState = 0;        // Holds the current pressed switch
#define switchLine1   27     // Switches are 1-4 corresponding to lines of the display
#define switchLine2   33
#define switchLine3   39
#define switchLine4   45
#define switchStart   26     // Start and Stop switches
#define switchStop    32

// SWITCH LED SETUP                        **Red wires**
#define switchLine1LED  25   // LEDs will light up only for switches that are available for functions 
#define switchLine2LED  31
#define switchLine3LED  37
#define switchLine4LED  43
#define switchStartLED  24
#define switchStopLED   30

// SWITCH GND SETUP
#define switchLine1Gnd   23     // Switches are 1-4 corresponding to lines of the display
#define switchLine2Gnd   29
#define switchLine3Gnd   35
#define switchLine4Gnd   41
#define switchStartGnd   22     // Start and Stop switches
#define switchStopGnd    28


// OTHER VARIABLES
int stopCommand = 0;        // 1 = stop command

//  ******************************************** VOID SETUP ********************************************

void setup() {

  // DISPLAY SETUP
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE); // Switch on the backlight
  lcd.setBacklight(HIGH);             //  Set backlight level
  lcd.begin(20, 4);                   // set up the LCD's number of columns and rows:

  // Qik MOTOR CONTROLLER SETUP
  Serial.begin(115200);
  Serial.println("qik 2s9v1 dual serial motor controller initializing");

  qik.init();

  Serial.print("Firmware version: ");
  Serial.write(qik.getFirmwareVersion());
  Serial.println();

  Serial.print("RESET"); // Print on unit reset
  Serial.println();

  // INPUT SETUP
  pinMode(liquidLevelUpper, INPUT_PULLUP);  // Input pin pulled LOW when fluid LOW

  // OUTPUT SETUP
  //pinMode(VFDStart, OUTPUT);
  //pinMode(VFDStop, OUTPUT);
  pinMode(airPump1, OUTPUT);
  pinMode(airPump2, OUTPUT);
  pinMode(largeFan, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(buzzerGnd, OUTPUT);      // Set as output and pulled low for Ground

  
  //digitalWrite(VFDStart, LOW);
  //digitalWrite(VFDStop, LOW);
  digitalWrite(airPump1, LOW);     // off
  digitalWrite(airPump2, LOW);     // off
  digitalWrite(largeFan, LOW);     // off
  digitalWrite(buzzer, LOW);       // off
  digitalWrite(buzzerGnd, LOW);    // Pin Pulled Low as Ground for Buzzer


  // SWITCHES SETUP
  pinMode(switchLine1, INPUT_PULLUP);
  pinMode(switchLine2, INPUT_PULLUP);
  pinMode(switchLine3, INPUT_PULLUP);
  pinMode(switchLine4, INPUT_PULLUP);
  pinMode(switchStart, INPUT_PULLUP);
  pinMode(switchStop, INPUT_PULLUP);

  // SWITCH LED SETUP
  pinMode(switchLine1LED, OUTPUT);  // Active HIGH
  pinMode(switchLine2LED, OUTPUT);  // Active HIGH
  pinMode(switchLine3LED, OUTPUT);
  pinMode(switchLine4LED, OUTPUT);
  pinMode(switchStartLED, OUTPUT);
  pinMode(switchStopLED, OUTPUT);

  // SWITCH GND SETUP
  pinMode(switchLine1Gnd, OUTPUT);  // Active HIGH
  pinMode(switchLine2Gnd, OUTPUT);  // Active HIGH
  pinMode(switchLine3Gnd, OUTPUT);
  pinMode(switchLine4Gnd, OUTPUT);
  pinMode(switchStartGnd, OUTPUT);
  pinMode(switchStopGnd, OUTPUT);

  // All pins Pulled LOW as Ground for switches
  digitalWrite(switchLine1Gnd, LOW);
  digitalWrite(switchLine2Gnd, LOW);
  digitalWrite(switchLine3Gnd, LOW);
  digitalWrite(switchLine4Gnd, LOW);
  digitalWrite(switchStartGnd, LOW);
  digitalWrite(switchStopGnd, LOW);


  splashScreen (); // Load splash screen

} // END OF SETUP LOOP

//  ******************************************** VOID LOOP ********************************************

void loop() {


  // LOAD THE MAIN MENU
  mainScreen();
  // Loads the main screen, allows the user to increment and decrement the run time
  // Leaving the function is caused by the START button


  // CLEAR THE ROOM, FILL THE RESERVIOR
  stopCommand = clearRoomFillReservior();
  // After the START button pressed in mainScreen(), display the LEAVE THE ROOM warning and fill the bowl
  // May return the STOP command. if so, shutdown
  if (stopCommand == 1) {
    stopCommand = 0;
    shutdown();
  }

  // LOAD THE STARTUP SCREEN, STARTUP THE PUMPS
  systemStartup();   // Loads system starting screens, starts up the pumps

  //  ******************************************** MAIN LOOP (SYSTEM RUNNING) ********************************************


  startTime = millis() - 2000; // 2 seconds removed due to delay above

  // Run loop

  while ((millis() - startTime) < runTimeSeconds * 1000  && stopCommand == 0) {

    //TIME REMAINING CALCULATIONS
    remainingMinutes = (runTimeSeconds - ((millis() - startTime) / 1000)) / 60;
    remainingSeconds = (runTimeSeconds - ((millis() - startTime) / 1000)) - remainingMinutes * 60;

    // DISPLAY TIME REMAINING
    lcd.setCursor(2, 1);
    lcd.print(remainingMinutes);
    lcd.print(":");
    if (remainingSeconds < 10) {
      lcd.print("0");
    }
    lcd.print(remainingSeconds);
    lcd.print(" ");

    // LIQUID LEVEL CONTROL
    if (digitalRead(liquidLevelUpper) == LOW && runTimeSeconds > reserviorDrainTime) {  // Fluid low and not close to shutdown time


      // Turn fill pump (and Switch 1 light) on for one second
      qik.setM1Speed(-103);         // Fill pump at partial speed
      digitalWrite(switchLine1LED, HIGH);     // Active HIGH
      delay(3000);
      qik.setM1Speed(0);          // Pump OFF
      digitalWrite(switchLine1LED, LOW);     // Active HIGH
     
    }

  } // END OF WHILE LOOP
  shutdown();

} //END OF VOID LOOP


//  ******************************************** FUNCTIONS ********************************************


void splashScreen() {
  // SWITCH ALL LEDs ON FOR SPLASH SCREEN
  // On to test functionality
  digitalWrite(switchLine1LED, HIGH);     // Active HIGH
  digitalWrite(switchLine2LED, HIGH);     // Active HIGH
  digitalWrite(switchLine3LED, HIGH);     // Active HIGH
  digitalWrite(switchLine4LED, HIGH);     // Active HIGH
  digitalWrite(switchStartLED, HIGH);     // Active HIGH
  digitalWrite(switchStopLED, HIGH);      // Active HIGH

  // DISPLAY LOAD - SPLASH SCREEN
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("WELCOME TO");               // Print a message to the LCD.
  lcd.setCursor(0, 1);
  lcd.print("NebuPure's MAG 50(R)");     // Print a message to the LCD.lcd.print("Subtract a Minute");   // Print a message to the LCD.
  lcd.setCursor(5, 3);
  lcd.print("WARMING UP");

  // SWITCH ALL LEDS OFF
  digitalWrite(switchLine1LED, LOW);     // Active HIGH
  digitalWrite(switchLine2LED, LOW);     // Active HIGH
  digitalWrite(switchLine3LED, LOW);     // Active HIGH
  digitalWrite(switchLine4LED, LOW);     // Active HIGH
  digitalWrite(switchStartLED, LOW);     // Active HIGH
  digitalWrite(switchStopLED, LOW);      // Active HIGH

  delay((splashScreenSeconds * 1000) - 750);
}



void mainScreen() {
  // DISPLAY LOAD - MAIN SCREEN
  lcd.clear();
  lcd.setCursor(1, 0 );
  lcd.print("MAG50 Time:");
  lcd.setCursor(12, 0);
  lcd.print(runTimeSeconds / 60);
  lcd.print(" min ");
  lcd.setCursor(0, 1);
  lcd.print("< Add a Minute");  // SwitchLine2
  lcd.setCursor(0, 2);
  lcd.print("< Subtract a Minute"); // SwitchLine3
  lcd.setCursor(2, 3);
  lcd.print("'START' to begin"); // SwitchStart

  // SWITCH LEDs ON (Main Screen)
  // Turn on 2,3,Start,STOP LEDs
  digitalWrite(switchLine1LED, LOW);     // Active LOW - kept HIGH
  digitalWrite(switchLine2LED, HIGH);     // Active LOW
  digitalWrite(switchLine3LED, HIGH);     // Active LOW
  digitalWrite(switchLine4LED, LOW);     // Active LOW
  digitalWrite(switchStartLED, HIGH);     // Active LOW
  digitalWrite(switchStopLED, LOW);      // Active LOW

  // READ SWITCH INPUTS
  switchState = 0;
  while (digitalRead(switchStart) != 0) { // Loops until SWITCH 5 (Start Button) Pressed

    // SWITCH 2 - INCREASE RUN TIME
    if (digitalRead(switchLine2) == 0 && runTimeSeconds < 36000) {  // If switchLine2 pressed AND runTimeSeconds not too high (10hrs)

      // add a minute to runtime
      runTimeSeconds = runTimeSeconds + 60;
      lcd.setCursor(12, 0);
      lcd.print(runTimeSeconds / 60);
      lcd.print(" min ");

      // beep
      digitalWrite(buzzer, HIGH);     // on
      delay(4);
      digitalWrite(buzzer, LOW);     // on

      // debounce
      delay(400);
    }

    // SWITCH 3 - REDUCE RUN TIME
    if (digitalRead(switchLine3) == 0 && runTimeSeconds > 60) {  // If SwitchLine3 pressed AND runTimeSeconds not under 1 minute

      // Subtract a minute from runtime
      runTimeSeconds = runTimeSeconds - 60;
      lcd.setCursor(12, 0);
      lcd.print(runTimeSeconds / 60);
      lcd.print(" min ");

      // beep
      digitalWrite(buzzer, HIGH);     // on
      delay(4);
      digitalWrite(buzzer, LOW);     // on

      // debounce
      delay(400);
    }

    // SWITCH 6 - STOP SWITCH
    if (digitalRead(switchStop) == 0) { //If STOP switch pressed, just reset the run time seconds to the default
      runTimeSeconds = defaultRunTimeSeconds;
      lcd.setCursor(12, 0);
      lcd.print(runTimeSeconds / 60);
      lcd.print(" min ");

    }        // END OF FUNCTION IF
  }         // END OF FUNCTION WHILE
}          // END OF VOID MAINSCREEN



int clearRoomFillReservior() {
  // clear room, fill reservior

  Serial.print("CLEAR FILL"); // *****************************
  Serial.println();

  // LOAD the LEAVE ROOM SCREEN
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("*Leave the Room Now*");
  lcd.setCursor(3, 3);
  lcd.print("'STOP' to End");

  // SWITCH LEDs ON (CLEAR Screen)
  // Turn on appropriate switch LEDs for the Clear Screen
  digitalWrite(switchLine1LED, LOW);     // Active LOW
  digitalWrite(switchLine2LED, LOW);     // Active LOW
  digitalWrite(switchLine3LED, LOW);     // Active LOW
  digitalWrite(switchLine4LED, LOW);     // Active LOW
  digitalWrite(switchStartLED, LOW);     // Active LOW
  digitalWrite(switchStopLED, HIGH);      // Active LOW

  stopCommand = 0;

  int x = roomClearSeconds;
  while (x > 0 && stopCommand == 0) {
    // Buzzer
    digitalWrite(buzzer, HIGH);     // on
    lcd.setBacklight(LOW);
    delay(50);
    digitalWrite(buzzer, LOW);     // on
    lcd.setBacklight(HIGH);
    delay(940);
    x = x - 1;
    // FIll up fluid ***********************************


    // Stop the room clear and exit if stop pressed
    if (digitalRead(switchStop) == 0) {
      stopCommand = 1;
      x = 0;
    }
  }                           // END OF WHILE
  return stopCommand;
}                            // END OF CLEAR ROOM FILL RESERVIOR



void systemStartup() {

  // LOAD STARTING SYSTEM SCREEN
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("* STARTING SYSTEM *");

  Serial.print("STARTUP");
  Serial.println();

  // START PUMPS/FANS
  // Start liquid circulation pump
  qik.setM0Speed(86);

  digitalWrite(airPump1, HIGH);     // Start air pump1 and delay
  delay(1500);
  digitalWrite(airPump2, HIGH);     // Start air pump2 and delay
  //delay(10000);
  //digitalWrite(VFDStart, HIGH);
  delay(2000);
  digitalWrite(largeFan, HIGH);     // Start fan

  //LOAD RUN SCREEN
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("NebuPure's MAG50(R)");
  lcd.setCursor(9, 1);
  lcd.print("Remaining");
  lcd.setCursor(1, 3);
  lcd.print("'STOP' to Pause/End");


  // SWITCH LEDs ON (Run Screen)
  // Turn on Stop LED
  digitalWrite(switchLine1LED, LOW);     // Active LOW
  digitalWrite(switchLine2LED, LOW);     // Active LOW
  digitalWrite(switchLine3LED, LOW);     // Active LOW
  digitalWrite(switchLine4LED, LOW);     // Active LOW
  digitalWrite(switchStartLED, LOW);     // Active LOW
  digitalWrite(switchStopLED, HIGH);      // Active LOW
}




void shutdown() {

  // LOAD SYSTEM SHUTDOWN SCREEN
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("* SYSTEM SHUTDOWN *");

  Serial.print(remainingSeconds);
  Serial.println();

  // SHUT DOWN PUMPS
  for (int i = -46; i <= 0; i++) {    // Stop liquid circulation pump
    qik.setM0Speed(i);
  }
  for (int i = -46; i <= 0; i++) {    // Stop fill pump
    qik.setM1Speed(i);
  }

  //digitalWrite(VFDStart, LOW);
  //delay(10000);
  digitalWrite(airPump1, LOW);          // Stop air pump 1
  delay(1000);
  digitalWrite(airPump2, LOW);          // Stop air pump 2
  delay(1000);
  digitalWrite(largeFan, LOW);          // Stop fan
  delay(1000);

  // Buzzer sounds for end of shutdown
  digitalWrite(buzzer, HIGH);
  delay(1000);
  digitalWrite(buzzer, LOW);
  delay(3000);

  // SUMMARY SCREEN
  if (remainingMinutes == 0 && remainingSeconds < 2) { // Normal shutdown

    // SWITCH LEDs ON
    // Only the STOP switch is lit
    digitalWrite(switchLine1LED, LOW);     // Active LOW
    digitalWrite(switchLine2LED, LOW);     // Active LOW
    digitalWrite(switchLine3LED, LOW);     // Active LOW
    digitalWrite(switchLine4LED, LOW);     // Active LOW
    digitalWrite(switchStartLED, LOW);     // Active LOW
    digitalWrite(switchStopLED, HIGH);      // Active LOW

    // SUMMARY SCREEN - Normal shutdown
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("NebuPure's MAG50(R)");
    lcd.setCursor(2, 1);
    lcd.print("Treatment Ended");
    lcd.setCursor(0, 3);
    lcd.print("'STOP' for Main Menu");
    delay(3000);

    //Delay until STOP switch pressed
    while (digitalRead(switchStop) != 0) {}
  }

  else { // Shutdown was manual

    // SWITCH LEDs ON (Summary Screen)
    // switchLine4 and STOP switches are lit
    digitalWrite(switchLine1LED, LOW);     // Active LOW
    digitalWrite(switchLine2LED, LOW);     // Active LOW
    digitalWrite(switchLine3LED, LOW);     // Active LOW
    digitalWrite(switchLine4LED, LOW);     // Active LOW
    digitalWrite(switchStartLED, HIGH);     // Active LOW
    digitalWrite(switchStopLED, HIGH);      // Active LOW


    // SUMMARY SCREEN - manual shutdown
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Ended with ");

    // Print remaining time
    lcd.setCursor(1, 1);
    lcd.print(remainingMinutes);
    lcd.print(":");
    if (remainingSeconds < 10) {
      lcd.print("0");
    }
    lcd.print(remainingSeconds);
    lcd.print(" ");


    lcd.print("Min. Remaining");
    lcd.setCursor(2, 2);
    lcd.print("START' to resume");
    lcd.setCursor(0, 3);
    lcd.print("'STOP' for Main Menu");

    // Summary Screen - Low Fluid Error Shutdown
    

    }


    delay(2000);  // replace with wait for STOP to be released ****************************

    int x = 0;
    while (x == 0) {
      if (digitalRead(switchStop) == 0) {
        runTimeSeconds = defaultRunTimeSeconds;
        x = 1;
      }
      if (digitalRead(switchStart) == 0) {
        runTimeSeconds = remainingMinutes * 60 + remainingSeconds;
        x = 1;
      }
    } // END OF WHILE LOOP
  } // END OF ELSE
// END OF VOID SHUTDOWN

