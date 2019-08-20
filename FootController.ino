/*
Based on:
4-Way Button: Click, Double-Click, Press+Hold, and Press+Long-Hold Test Sketch
By Jeff Saltzman, Oct. 13, 2009

Changes:
-  support up to 4 buttons (active Low)
-  Removed double-click
-  Removed very long hold
-  Added Midi control messages


Initial setting:

Preset mode:
1) Preset Down
2) Preset Up
3) Snapshot Up  
4) Tap Tempo

Snapshot mode:
1) Snapshot 1
2) Snapshot 2
3) Snapshot 3  
4) Snapshot 4

Stomp mode:
1)Stomp 1 on/off
2)Stomp 2 on/off
3)Stomp 3 on/off
4)Stomp 4 on/off / Tuner Off in tuner mode

Press and hold:
1) Preset Mode
2) Snapshot Mode
3) Stomp Mode
4) Tuner On
*/

#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

#define butPin1 2 // digital input for button 1
#define butPin2 3 // digital input for button 2
#define butPin3 4 // digital input for button 3
#define butPin4 5 // digital input for button 4
#define ledPin1 6 // digital output pin for LED 1
#define ledPin2 7 // digital output pin for LED 2
#define ledPin3 8 // digital output pin for LED 3
#define ledPin4 9 // digital output pin for LED 4
#define modePin1 12 // digital output pin for mode 1
#define modePin2 11 // digital output pin for mode 2
#define modePin3 10 // digital output pin for mode 3

// LED variables
boolean ledVal1 = true; // state of LED 1
boolean ledVal2 = true; // state of LED 2
boolean ledVal3 = true; // state of LED 3
boolean ledVal4 = true; // state of LED 4

// Button definitions
//int buttonPin = 0;
int buttonPin1 = 2;
int buttonPin2 = 3;
int buttonPin3 = 4;
int buttonPin4 = 5;

// Button timing variables
int debounce = 50; // ms debounce period to prevent flickering when pressing or releasing the button
int DCgap = 50; // max ms between clicks for a double click event
int holdTime = 500; // ms hold period: how long to wait for press+hold event
int longHoldTime = 5000; // ms long hold period: how long to wait for press+hold event
int b;

// Other button variables
boolean buttonVal1 = HIGH; // value read from button
boolean buttonVal2 = HIGH; // value read from button
boolean buttonVal3 = HIGH; // value read from button
boolean buttonVal4 = HIGH; // value read from button
boolean buttonLast1 = HIGH; // buffered value of the button's previous state
boolean buttonLast2 = HIGH; // buffered value of the button's previous state
boolean buttonLast3 = HIGH; // buffered value of the button's previous state
boolean buttonLast4 = HIGH; // buffered value of the button's previous state
boolean DCwaiting1 = false; // whether we're waiting for a double click (down)
boolean DCwaiting2 = false;
boolean DCwaiting3 = false;
boolean DCwaiting4 = false;
boolean DConUp1 = false; // whether to register a double click on next release, or whether to wait and click
boolean DConUp2 = false;
boolean DConUp3 = false;
boolean DConUp4 = false;
boolean singleOK1 = true; // whether it's OK to do a single click
boolean singleOK2 = true;
boolean singleOK3 = true;
boolean singleOK4 = true;
long downTime1 = -1; // time the button was pressed down
long downTime2 = -1;
long downTime3 = -1;
long downTime4 = -1;
long upTime1 = -1; // time the button was released
long upTime2 = -1;
long upTime3 = -1;
long upTime4 = -1;
boolean ignoreUp1 = false; // whether to ignore the button release because the click+hold was triggered
boolean ignoreUp2 = false;
boolean ignoreUp3 = false;
boolean ignoreUp4 = false;
boolean waitForUp1 = false; // when held, whether to wait for the up event
boolean waitForUp2 = false;
boolean waitForUp3 = false;
boolean waitForUp4 = false;
boolean holdEventPast1 = false; // whether or not the hold event happened already
boolean holdEventPast2 = false;
boolean holdEventPast3 = false;
boolean holdEventPast4 = false;
boolean longHoldEventPast1 = false;// whether or not the long hold event happened already
boolean longHoldEventPast2 = false;
boolean longHoldEventPast3 = false;
boolean longHoldEventPast4 = false;

// Midi parameters
int SYSEX1 = 0xF0;
int SYSEX2 = 0xF7;
int ID1 = 0x00;
int ID2 = 0x01;
int ID3 = 0x0C;
int IDAX = 00;
int PAREDIT = 0x04;
int CC = 0xB0;
int PP = 0xC0;
int SWITCH = 0x81; 
byte commandByte;
byte noteByte;
byte velocityByte;
int St;
int Mo;
int Dl;
int Rv;

int cc_fs1 = 49;       // 0 - 127
int cc_fs2 = 50;
int cc_fs3 = 51;
int cc_fs4 = 52;
int cc_fs5 = 53;
int cc_fs7 = 54;
int cc_fs8 = 55;
int cc_fs9 = 56;
int cc_fs10 = 57;
int cc_fs11 = 58;
int cc_taptempo = 64;  // 64-127
int cc_tuner = 68;
int cc_snapshot = 69;  // Snapshot-keuze (0= Snapshot 1, 1= Snapshot 2, 2= Snapshot 3, 8= next, 9= previous
int cc_stompmode = 71; // Voetschakelaarmode (0= Stomp, 1= Scroll, 2= Preset, 3= Snapshot, 4= next, 5= previous)

int MODE = 1;          // 1=preset 2=snapshot 3=stomp
int TEMPO = 64;
int TEMPCOUNTER = 0;
int TEMPSTART = 0;
int ShowTunerPage = 0;
int CHAN = 1;
int PC = 0;
int SNAP = 0;
int fs4 = 0;
int fs5 = 0;
int fs6 = 0;
int fs7 = 0;



// The procedures
int checkButton(int button)
{ 
int event = 0;
  // Read the state of the button
  switch (button) {
     case 2: // ----Procedure for button 1 ----
     buttonVal1 = digitalRead(butPin1);
  if (buttonVal1 == LOW && buttonLast1 == HIGH && (millis() - upTime1) > debounce) {
    downTime1 = millis();
    ignoreUp1 = false;
    waitForUp1 = false;
    singleOK1 = true;
    holdEventPast1 = false;
    longHoldEventPast1 = false;
    if ((millis()-upTime1) < DCgap && DConUp1 == false && DCwaiting1 == true) DConUp1 = true;
    else DConUp1 = false;
    DCwaiting1 = false;
  }
  
// Button released
  else if (buttonVal1 == HIGH && buttonLast1 == LOW && (millis() - downTime1) > debounce) { 
    if (not ignoreUp1) {
      upTime1 = millis();
      if (DConUp1 == false) DCwaiting1 = true;
    else {
      event = 2;
      DConUp1 = false;
      DCwaiting1 = false;
      singleOK1 = false;
      }
    }
  }
  
// Test for normal click event: DCgap expired
  if ( buttonVal1 == HIGH && (millis()-upTime1) >= DCgap && DCwaiting1 == true && DConUp1 == false && singleOK1 == true) {
      event = 1;
      DCwaiting1 = false;
    }
// Test for hold
  if (buttonVal1 == LOW && (millis() - downTime1) >= holdTime) {
// Trigger "normal" hold
  if (not holdEventPast1) {
    event = 3;
    waitForUp1 = true;
    ignoreUp1 = true;
    DConUp1 = false;
    DCwaiting1 = false;
//downTime = millis();
    holdEventPast1 = true;
    }
  }  
    buttonLast1 = buttonVal1;   
   return event;
   break;
   
     case 3: // ----Procedure for button 2 ----
     buttonVal2 = digitalRead(butPin2);
     if (buttonVal2 == LOW && buttonLast2 == HIGH && (millis() - upTime2) > debounce) {
    downTime2 = millis();
    ignoreUp2 = false;
    waitForUp2 = false;
    singleOK2 = true;
    holdEventPast2 = false;
    longHoldEventPast2 = false;
    if ((millis()-upTime2) < DCgap && DConUp2 == false && DCwaiting2 == true) DConUp2 = true;
    else DConUp2 = false;
    DCwaiting2 = false;
  }
  
// Button released
  else if (buttonVal2 == HIGH && buttonLast2 == LOW && (millis() - downTime2) > debounce) { 
    if (not ignoreUp2) {
      upTime2 = millis();
      if (DConUp2 == false) DCwaiting2 = true;
    else {
      event = 2;
      DConUp2 = false;
      DCwaiting2 = false;
      singleOK2 = false;
      }
    }
  }
  
// Test for normal click event: DCgap expired
  if ( buttonVal2 == HIGH && (millis()-upTime2) >= DCgap && DCwaiting2 == true && DConUp2 == false && singleOK2 == true) {
      event = 1;
      DCwaiting2 = false;
    }
// Test for hold
  if (buttonVal2 == LOW && (millis() - downTime2) >= holdTime) {
// Trigger "normal" hold
  if (not holdEventPast2) {
    event = 3;
    waitForUp2 = true;
    ignoreUp2 = true;
    DConUp2 = false;
    DCwaiting2 = false;
//downTime = millis();
    holdEventPast2 = true;
    }
  }  
    buttonLast2 = buttonVal2;  
   return event;
   break;
  
     case 4:   // ----Procedure for button 3 ----
     buttonVal3 = digitalRead(butPin3);
     if (buttonVal3 == LOW && buttonLast3 == HIGH && (millis() - upTime3) > debounce) {
    downTime3 = millis();
    ignoreUp3 = false;
    waitForUp3 = false;
    singleOK3 = true;
    holdEventPast3 = false;
    longHoldEventPast3 = false;
    if ((millis()-upTime3) < DCgap && DConUp3 == false && DCwaiting3 == true) DConUp3 = true;
    else DConUp3 = false;
    DCwaiting3 = false;
  }
  
// Button released
  else if (buttonVal3 == HIGH && buttonLast3 == LOW && (millis() - downTime3) > debounce) { 
    if (not ignoreUp3) {
      upTime3 = millis();
      if (DConUp3 == false) DCwaiting3 = true;
    else {
      event = 2;
      DConUp3 = false;
      DCwaiting3 = false;
      singleOK3 = false;
      }
    }
  }
  
// Test for normal click event: DCgap expired
  if ( buttonVal3 == HIGH && (millis()-upTime3) >= DCgap && DCwaiting3 == true && DConUp3 == false && singleOK3 == true) {
      event = 1;
      DCwaiting3 = false;
    }
// Test for hold
  if (buttonVal3 == LOW && (millis() - downTime3) >= holdTime) {
// Trigger "normal" hold
  if (not holdEventPast3) {
    event = 3;
    waitForUp3 = true;
    ignoreUp3 = true;
    DConUp3 = false;
    DCwaiting3 = false;
//downTime = millis();
    holdEventPast3 = true;
    }
  }  
     buttonLast3 = buttonVal3;   
   return event;
   break;
   
     case 5: // ----Procedure for button 4 ----
     buttonVal4 = digitalRead(butPin4);
     if (buttonVal4 == LOW && buttonLast4 == HIGH && (millis() - upTime4) > debounce) {
    downTime4 = millis();
    ignoreUp4= false;
    waitForUp4 = false;
    singleOK4 = true;
    holdEventPast4 = false;
    longHoldEventPast4 = false;
    if ((millis()-upTime4) < DCgap && DConUp4 == false && DCwaiting4 == true) DConUp4 = true;
    else DConUp4 = false;
    DCwaiting4 = false;
  }
  
// Button released
  else if (buttonVal4 == HIGH && buttonLast4 == LOW && (millis() - downTime4) > debounce) { 
    if (not ignoreUp4) {
      upTime4 = millis();
      if (DConUp4 == false) DCwaiting4 = true;
    else {
      event = 2;
      DConUp4 = false;
      DCwaiting4 = false;
      singleOK4 = false;
      }
    }
  }
  
// Test for normal click event: DCgap expired
  if ( buttonVal4 == HIGH && (millis()-upTime4) >= DCgap && DCwaiting4 == true && DConUp4 == false && singleOK4 == true) {
      event = 1;
      DCwaiting4 = false;
    }
// Test for hold
  if (buttonVal4 == LOW && (millis() - downTime4) >= holdTime) {
// Trigger "normal" hold
  if (not holdEventPast4) {
    event = 3;
    waitForUp4 = true;
    ignoreUp4 = true;
    DConUp4 = false;
    DCwaiting4 = false;
//downTime = millis();
    holdEventPast4 = true;
    }
  }  
      buttonLast4 = buttonVal4;   
  return event;
  break;
  }
  // Button pressed down

}

//=================================================
// Events to trigger by click and press+hold

void clickEvent(int stomp) {
// What happens: Program change 
  switch (MODE) {
// preset mode
    case 1:  //Preset select
    
    switch (stomp) {
     case 2: 
      digitalWrite(ledPin1, 0);
      PC = --PC;
      if (PC < 0) {
          PC = 24;
      }
//      SendCC(CHAN, 32, 3);
      SendPC(CHAN, PC);
      fs4 = 0;                  
      fs5 = 0; 
      digitalWrite(ledPin1, 1);
      break;
      
    case 3: 
      digitalWrite(ledPin2, 0);
      PC = ++PC;
      if (PC > 23) {
          PC = 0;
      }
//      SendCC(CHAN, 32, 3);
      SendPC(CHAN, PC);
      fs4 = 0;                  
      fs5 = 0; 
      digitalWrite(ledPin2, 1);
      break;

    case 4: 
      digitalWrite(ledPin3, 0);
      SendCC(CHAN, cc_snapshot, SNAP);
      SNAP = ++SNAP;
      if (SNAP > 3) {
          SNAP = 0;
      }
      fs4 = 0;                  
      fs5 = 0; 
      digitalWrite(ledPin3, 1);
      break;
      
    case 5:
      digitalWrite(ledPin4, 0);
      SendCC(CHAN, cc_taptempo, 100);
      digitalWrite(ledPin4, 1);
      break;
  
    }
    break;
    
   case 2: //select snapshots
    switch (stomp) {  

     case 2: 
       SendCC(CHAN, cc_snapshot, 0);
       break;
    
    case 3: 
       SendCC(CHAN, cc_snapshot, 1);
       break;
    
    case 4: 
       SendCC(CHAN, cc_snapshot, 2);
       break;
    
    case 5:
       SendCC(CHAN, cc_snapshot, 3);
       break;      
    }
    break;    
    
   case 3: 
    switch (stomp) {
     case 2: 
       if (fs4 == 0) {
           SendCC(CHAN, cc_fs4, 127);
           digitalWrite(ledPin1, 0);
           fs4 = 1;
       } else {
           SendCC(CHAN, cc_fs4, 0);
           digitalWrite(ledPin1, 1);
           fs4 = 0;                  
       }
       break;
    
    case 3: 
       if (fs5 == 0) {
           SendCC(CHAN, cc_fs5, 127);
           digitalWrite(ledPin2, 0);
           fs5 = 1;
       } else {
           SendCC(CHAN, cc_fs5, 0);
           digitalWrite(ledPin2, 1);
           fs5 = 0;                  
       }
       break;
    
    case 4: 
      digitalWrite(ledPin3, 0);
      SendCC(CHAN, cc_taptempo, 100);
      digitalWrite(ledPin3, 1);
      break;
    
    case 5:
       if (ShowTunerPage == 0) {
           SendCC(CHAN, cc_tuner, 127);
           digitalWrite(ledPin4, 0);
           ShowTunerPage = 1;
       } else {
           SendCC(CHAN, cc_tuner, 0);
           ShowTunerPage = 0;
           digitalWrite(ledPin4, 1);
       }
       break;      
    }
    break;
  }
}

void holdEvent(int pin) {
// What happens: Switch mode     
    switch (pin) {
     case 2:  // mode 1 
          digitalWrite(modePin1, 0);
          digitalWrite(modePin2, 1);
          digitalWrite(modePin3, 1);
          digitalWrite(ledPin1, 1);
          digitalWrite(ledPin2, 1);
          digitalWrite(ledPin3, 1);
          digitalWrite(ledPin4, 1);
//          fs4 = 0;                  
//          fs5 = 0;                  
          ShowTunerPage = 0;  
          MODE = 1;          
      break;
      
    case 3: 
          digitalWrite(modePin1, 1);
          digitalWrite(modePin2, 0);
          digitalWrite(modePin3, 1);   
          digitalWrite(ledPin1, 1);
          digitalWrite(ledPin2, 1);
          digitalWrite(ledPin3, 1);
          digitalWrite(ledPin4, 1);
//          fs4 = 0;                  
//          fs5 = 0;                  
          ShowTunerPage = 0;         
          MODE = 2;          
      break;
    
    case 4:
          digitalWrite(modePin1, 1);
          digitalWrite(modePin2, 1);
          digitalWrite(modePin3, 0); 
          digitalWrite(ledPin1, 1);
          digitalWrite(ledPin2, 1);
          digitalWrite(ledPin3, 1);
          digitalWrite(ledPin4, 1);
//          fs4 = 0;                  
//          fs5 = 0;                  
          ShowTunerPage = 0;
          if (fs4 == 0) {
           digitalWrite(ledPin1, 1);
          } else {
           digitalWrite(ledPin1, 0);
          }           
          if (fs5 == 0) {
           digitalWrite(ledPin2, 1);
          } else {
           digitalWrite(ledPin2, 0);
          } 
          MODE = 3;          
      break;
    
    case 5:
      if (ShowTunerPage == 1) {
         SendCC(CHAN, cc_tuner, 0);
         ShowTunerPage = 127;
      } else {
         SendCC(CHAN, cc_tuner, 127);
         ShowTunerPage = 0;
      }
      break;  
    }
}

void longHoldEvent(int pin) {
// What happens: Tuner
  ledVal4 = !ledVal4;
  digitalWrite(ledPin4, ledVal4);
}

uint8_t MIDI_calc_Roland_checksum(uint16_t sum) {
  uint8_t checksum = 0x80 - (sum % 0x80);
  if (checksum == 0x80) checksum = 0;
  return checksum;
}

void SendSysex(int ad1, int ad0, int DATA_IN) {
//  byte *ad = (byte*)&INPIN; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
//  byte ad3 = 0x18;
//  byte ad2 = 0x00;
  int checksum = MIDI_calc_Roland_checksum(0x18 + 0x00 + ad1 + ad0 + DATA_IN); // Calculate the Roland checksum
//  byte checksum = 0x00;
  byte sysexmessage[] = { 0x41, 0x10, 0x00, 0x00, 0x53, 0x12, 0x18, 0x00, ad1, ad0, DATA_IN, checksum };
  MIDI.sendSysEx(12, sysexmessage, false);
  delay(100);
}

void SendCC(int chan, int cc, int val) {
  MIDI.sendControlChange(cc, val, chan);
}

void SendPC(int chan, int prg) {
  MIDI.sendProgramChange(prg, chan);
}

//====the main=============================================

void setup() 
{
// Set button input pin
  pinMode(buttonPin1, INPUT);
  digitalWrite(buttonPin1, HIGH );
  pinMode(buttonPin2, INPUT);
  digitalWrite(buttonPin2, HIGH );
  pinMode(buttonPin3, INPUT);
  digitalWrite(buttonPin3, HIGH );
  pinMode(buttonPin4, INPUT);
  digitalWrite(buttonPin4, HIGH );
// Set LED output pins
  pinMode(ledPin1, OUTPUT);
  digitalWrite(ledPin1, ledVal1);
  pinMode(ledPin2, OUTPUT);
  digitalWrite(ledPin2, ledVal2);
  pinMode(ledPin3, OUTPUT); 
  digitalWrite(ledPin3, ledVal3);
  pinMode(ledPin4, OUTPUT); 
  digitalWrite(ledPin4, ledVal4);
  pinMode(modePin1, OUTPUT);
  pinMode(modePin2, OUTPUT);
  pinMode(modePin3, OUTPUT);
  digitalWrite(modePin1, 0);
  digitalWrite(modePin2, 1);
  digitalWrite(modePin3, 1);    
// midi config
  MIDI.begin(MIDI_CHANNEL_OMNI);
  SendPC(CHAN, 0);
}

void loop()
{
// Get button event and act accordingly 
     b = checkButton(2);
     if (b == 1) clickEvent(2);
     if (b == 3) holdEvent(2);
     b = checkButton(3);
     if (b == 1) clickEvent(3);
     if (b == 3) holdEvent(3);
     b = checkButton(4);
     if (b == 1) clickEvent(4);
     if (b == 3) holdEvent(4);
     b = checkButton(5);
     if (b == 1) clickEvent(5);
     if (b == 3) holdEvent(5); 
}
