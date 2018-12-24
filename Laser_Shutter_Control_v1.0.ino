/* 
Shutter control V0.9 10/01/2017 
The code controls an LCD screen and opens and closes a shutter.
It has a manual mode and a preset mode which also includes a wait
It relies upon a transistor circut to open the shutter which is controlled by a DC motor
It use pin 2 as the output pin to control a realy
*/

/* 
Shutter control V1.0 04/06/2017 
The code controls an LCD screen and opens and closes a shutter.
It has a manual mode and a preset mode which also includes a wait
It relies upon a stepper motor control board 
The stepper motor 28BYJ-48
The stepper motor turns 90 degress to open then 90 degrees to close
It uses four pins to control the servo a function called...  controls the servo
*/

/*
Shutter control V1.1 16/12/2018
change the delay so every push is 30 seconds
add piezo buzzer to indicate close shutter
 */


#include <SoftwareSerial.h>
#include <LiquidCrystal.h>          // LCD libray 
#include <Stepper.h>                // Stepper libray



//---( Number of steps per revolution of INTERNAL motor in 4-step mode )---
int  STEPS_PER_MOTOR_REVOLUTION = 32 ;

//---( Steps per OUTPUT SHAFT of gear reduction )---
int  STEPS_PER_OUTPUT_REVOLUTION = (32 * 64) ; //2048  


int SetPointTime = 10;              // How long shutter is open in seconds
int WaitTime = 30 ;                 // How long to wait in seconds

int Steps2Take;                     // Varaiable controlling number of steps
                                    // this will be a fraction of 2048           


int state = 1023;                   // This is the no button state
String ShutterModeName = "Manual" ; // This can be "Manual" or "Auto" 
int mode;                           // 0 is manual mode and 1 is auto mode
int AutoModeVariable = 0 ;          // 0 is the wait, 1 is the setpoint, 2 is execute.

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// initialize a stepper object,  (can't use pins 8 and 9)
Stepper small_stepper(STEPS_PER_MOTOR_REVOLUTION, 3, 12, 11, 13);  //3, 12 11, 13

const int buzzer = 2;               // The pin the buzzer is connected to.

void setup() {
  //setup serial
  Serial.begin(9600);     // Serial is only used during code de-bugging
  pinMode(0, INPUT);      // Pin Mode 0 is used for the buttons, all button inputs are detected on pin 0. Each button has a voltage range
  lcd.begin(16, 2);       // set up the LCD's number of columns and rows:
  ChangeMode(0);          // run the shutter mode function on start up and set it to manual mode to start with
  
  pinMode(2,OUTPUT);       // Buzzer pin set to output
  
  }


//The loop just waits for a button press to occur. The default button state is 1023. So if this is detected loop does nothing.

void loop() {
    
   if (analogRead(0) < 1000) { 
     state = analogRead(0);                 // Set the state variable to whatever button has been pressed
                              
     Serial.print("button press ");         // Some debugging if needed
     Serial.println(state);
     ButtonPress ();                        // Then go to the ButtonPress function to find out which button has been pressed            
     }
Serial.print("waiting in main loop: mode is  "); Serial.println(mode); delay(500);  // some debugging
}
//end of loop

int ButtonPress () {      // This just detects the button press and decides which button has been pressed
delay(500);               // allow for depress of button
// up button = 99;
// down button = 254;
// right button = 0;
  
    
/* select button */   if (state >= 637 && state <= 642) { if (mode == 0) { ChangeMode(1);} else {SetupWait();}} // The select button changes from manual to auto, if already auto then changes the auto settings.
/* up button */          else if (state >= 97 && state <= 101) {if (mode == 0){ lcd.print("open ");  Serial.println (" open ") ; openShutter() ; TimeOpen() ; lcd.setCursor(0,1);}}
             
}



// Manual Time open Counter in seconds
//This is the easiest way to open the shutter, it just waits for open button the waits for close button

int TimeOpen () {                               // When in manual mode and the up button is detected run this code untill the down button is detected
Serial.println("manual shutter open");          // Some debugging
int currentSeconds = millis()/1000;             // Set an inter to the current number of seconds since the code started

do {                                            //
lcd.setCursor(0,1);
state = analogRead(0);
lcd.print("Shutter Open   s");
int timeOpen = (millis()/1000) - currentSeconds;
lcd.setCursor(13,1);
lcd.print(timeOpen);
lcd.setCursor(13,1);
delay(1000);                                     // stops the display from shimmering   
Serial.println(timeOpen);                        // Some debugging
Serial.println(state);
}                                               
while (state <= 252 || state >= 255)  ;         // Here is the condition which is tested, as long as the down variable is not the down button thn keep going
//digitalWrite(2, LOW) ;                           // As soon as down is detected stop and close the shutter
closeShutter();
ChangeMode(0);                                   // Set the mode back to manual
}  


//This function sets up the screen for the atuo shutter open
int ChangeMode(int Smode) {
 if (Smode == 0) { 
 ShutterModeName = "Manual mode" ;
 lcd.setCursor(0, 0);
 lcd.print(ShutterModeName);       //The code starts in manual mode, 
 lcd.setCursor(0,1);
 lcd.print("Shutter closed   ");
 mode = 0;
 } 
 else if (Smode == 1){
 ShutterModeName = "Auto" ;
      
 lcd.setCursor(0, 0);
 lcd.print(ShutterModeName);       
 lcd.print("   Wait 30 s");
 lcd.setCursor(0,1);
 lcd.print("Set 10 s  Closed ");
 lcd.setCursor(12,0);
 lcd.print(WaitTime);
 lcd.setCursor(0,0);              
 lcd.blink();
 mode = 1;                        // Then return to the main loop to wait for next button press
}
}

int SetupWait () {
Serial.println("Enter Setup wait");
delay(1000);                      // Pause so you don't skip ahead
lcd.setCursor(12,0);              // Place the cursor at the wait variable              
lcd.blink();
do {
state = analogRead(0);
delay(500);                                      // Wait for button to depress
if (state == 99) { WaitTime = (WaitTime + 30) ;} 
   else if (state >= 252 && state <= 255) { WaitTime = (WaitTime - 30) ;}
//Serial.println(WaitTime);
lcd.print(WaitTime);
lcd.setCursor(12,0);  
}
while (state <= 637 || state >= 642)  ;     // Here is the condition which is tested, as long as the select button is not pushed then keep going
Serial.print("Exit Setup wait, wait time set to "); Serial.println(WaitTime);
SetupTime();
}  


int SetupTime() {
Serial.println("Enter Setup time");   // Some debugging
lcd.setCursor(4,1);
lcd.print(SetPointTime);// Place the cursor at the wait variable              
lcd.setCursor(4,1);
lcd.blink();
do {
state = analogRead(0);
delay(500);                                      // Wait for button to depress
if (state >= 97 && state <= 101)                 // If up buttom pressed
{ SetPointTime = (SetPointTime + 1) ; lcd.print(SetPointTime); lcd.setCursor(4,1);}
 
   else if (state >= 252 && state <= 255)        // If down button pressed
   { SetPointTime = (SetPointTime - 1) ;lcd.print(SetPointTime); lcd.setCursor(4,1);
   if (SetPointTime < 10) { lcd.setCursor(5,1); lcd.print(" ") ; lcd.setCursor(4,1); }
   }


 
}
while (state <= 637 || state >= 643)  ;     // Here is the condition which is tested, as long as the select button is not pushed then keep going
Serial.print("Exit Setup time, Setuptime set to "); Serial.println(SetPointTime); 
lcd.setCursor(10,1);                             // Place the cursor at "closed" ready to open
OpenAuto();

}

int OpenAuto() {
Serial.print("Enter Open Auto time set to "); Serial.println(SetPointTime); 
delay(500);
do {
state = analogRead(0);
if (state >= 97 && state <= 101) { 
Serial.println("Start Auto Open");
lcd.setCursor(12,0);
lcd.noBlink();
for (int i = WaitTime; i > 0; i--) 
     { lcd.print(i); lcd.print(" "); Serial.println(i); delay(1000); lcd.setCursor(12,0);  //Write the countdown wait time plus a space then reset cursor
        }
lcd.print(WaitTime);          // just write the wait time back again to keep it tidy
delay(500);

//digitalWrite(2, HIGH) ;
openShutter();
lcd.setCursor(10,1);
lcd.print("open  ")  ;
lcd.setCursor(4,1);


for (int j = SetPointTime; j > 0; j--) 
     { lcd.print(j); lcd.print(" "); Serial.println(j); delay(1000); lcd.setCursor(4,1); //Write the countdown open time plus a space then reset cursor
        }
//digitalWrite(2, LOW) ;
closeShutter();
lcd.setCursor(10,1);
lcd.print("Closed");                      // Write closed after shutter closed
lcd.setCursor(4,1);                       // Then write back open time 
lcd.print(SetPointTime);
state = 254;
Serial.println("Finished auto open");
}
  
}

while (state <= 637 || state >= 642)  ;     // Here is the condition which is tested, as long as the select button is not pushed then keep going
lcd.setCursor(0,0); 
}

//Functions for opening and closing the shutter
void openShutter ()  {
  //Steps2Take  =  STEPS_PER_OUTPUT_REVOLUTION / 2;  // Rotate CW 1/2 turn
  small_stepper.setSpeed(700);   
  small_stepper.step(512);
  delay(1000);  
}

void closeShutter ()  {
  //Steps2Take  =  -STEPS_PER_OUTPUT_REVOLUTION / 2;  // Rotate CW 1/2 turn
  small_stepper.setSpeed(700);   
  small_stepper.step(-700);
  delay(1000);
  tone(buzzer,2000);
  delay(1000);
  noTone(buzzer);
  delay(2000);
  tone(buzzer,2000);
  delay(1000);
  noTone(buzzer);
  delay(2000);
  tone(buzzer,2000);
  delay(1000);
  noTone(buzzer); 
  
}
