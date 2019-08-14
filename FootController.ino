/*
4-Way Button: Click, Double-Click, Press+Hold, and Press+Long-Hold Test Sketch
By Jeff Saltzman
Oct. 13, 2009

To keep a physical interface as simple as possible, this sketch demonstrates generating four output events from a single push-button.
1) Click: rapid press and release
2) Double-Click: two clicks in quick succession
3) Press and Hold: holding the button down
4) Long Press and Hold: holding the button down for a long time 

Initial setting:

Click:
1) Preset Down
2) Preset Up
3) Snapshot Up  
4) Tap Tempo

Click:
1)Stomp 1 on/off
2)Stomp 2 on/off
3)Stomp 3 on/off
4)Stomp 4 on/off / Tuner Off in tuner mode

Double click:
1) Prev Preset
2) Next Preset
3) Prev Snapshot
4) Next Snapshot

Press and hold:
1) Preset Mode
2) Scroll Mode
3) Stomp Mode
4) Tuner On

Long press and hold:
1)
2)
3)
4)


*/

#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

#define ledPin1 6 // digital output pin for LED 1
#define ledPin2 7 // digital output pin for LED 2
#define ledPin3 8 // digital output pin for LED 3
#define ledPin4 9 // digital output pin for LED 4
#define modePin1 10 // digital output pin for mode 1
#define modePin2 11 // digital output pin for mode 2
#define modePin3 12 // digital output pin for mode 3

// LED variables
boolean ledVal1 = false; // state of LED 1
boolean ledVal2 = false; // state of LED 2
boolean ledVal3 = false; // state of LED 3
boolean ledVal4 = false; // state of LED 4

// Button definitions
int buttonPin = 0;
int buttonPin1 = 2;
int buttonPin2 = 3;
int buttonPin3 = 4;
int buttonPin4 = 5;

// Button timing variables
int debounce = 20; // ms debounce period to prevent flickering when pressing or releasing the button
int DCgap = 50; // max ms between clicks for a double click event
int holdTime = 1000; // ms hold period: how long to wait for press+hold event
int longHoldTime = 5000; // ms long hold period: how long to wait for press+hold event

// Other button variables
boolean buttonVal = HIGH; // value read from button
boolean buttonLast = HIGH; // buffered value of the button's previous state
boolean DCwaiting = false; // whether we're waiting for a double click (down)
boolean DConUp = false; // whether to register a double click on next release, or whether to wait and click
boolean singleOK = true; // whether it's OK to do a single click
long downTime = -1; // time the button was pressed down
long upTime = -1; // time the button was released
boolean ignoreUp = false; // whether to ignore the button release because the click+hold was triggered
boolean waitForUp = false; // when held, whether to wait for the up event
boolean holdEventPast = false; // whether or not the hold event happened already
boolean longHoldEventPast = false;// whether or not the long hold event happened already

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
unsigned long StartTime;
unsigned long CurrentTime;
unsigned long ElapsedTime;

int ShowTunerPage = 0;
int CHAN = 1;
int PC = 0;
int SNAP = 0;
int fs7 = 0;
int fs8 = 0;
int fs9 = 0;
int fs10 = 0;



// The procedures
int checkButton(int pin)
{ 
int event = 0;
  // Read the state of the button
  buttonVal = digitalRead(pin);
  // Button pressed down
  if (buttonVal == LOW && buttonLast == HIGH && (millis() - upTime) > debounce) {
    downTime = millis();
    ignoreUp = false;
    waitForUp = false;
    singleOK = true;
    holdEventPast = false;
    longHoldEventPast = false;
    if ((millis()-upTime) < DCgap && DConUp == false && DCwaiting == true) DConUp = true;
    else DConUp = false;
    DCwaiting = false;
  }
  
// Button released
  else if (buttonVal == HIGH && buttonLast == LOW && (millis() - downTime) > debounce) { 
    if (not ignoreUp) {
      upTime = millis();
      if (DConUp == false) DCwaiting = true;
    else {
      event = 2;
      DConUp = false;
      DCwaiting = false;
      singleOK = false;
      }
    }
  }
  
// Test for normal click event: DCgap expired
  if ( buttonVal == HIGH && (millis()-upTime) >= DCgap && DCwaiting == true && DConUp == false && singleOK == true) {
      event = 1;
      DCwaiting = false;
    }
// Test for hold
  if (buttonVal == LOW && (millis() - downTime) >= holdTime) {
// Trigger "normal" hold
  if (not holdEventPast) {
    event = 3;
    waitForUp = true;
    ignoreUp = true;
    DConUp = false;
    DCwaiting = false;
//downTime = millis();
    holdEventPast = true;
    }
// Trigger "long" hold
  if ((millis() - downTime) >= longHoldTime) {
    if (not longHoldEventPast) {
      event = 4;
      longHoldEventPast = true;
      }
    }
  }
  buttonLast = buttonVal;
  return event;
}

//=================================================
// Events to trigger by click and press+hold

void clickEvent(int pin) {
// What happens: Program change 
  ledVal1 = !ledVal1;
  digitalWrite(ledPin1, ledVal1);
  switch (MODE) {
// preset mode
    case 1:  //Preset select
    switch (pin) {
     case 1: 
      PC = --PC;
      if (PC < 0) {
          PC = 24;
      }
      SendPC(CHAN, PC);
      break;
    case 2: 
      PC = ++PC;
      if (PC > 24) {
          PC = 0;
      }
      SendPC(CHAN, PC);
      break;
    case 3: 
      SendCC(CHAN, cc_snapshot, SNAP);
      SNAP = ++SNAP;
      if (SNAP > 4) {
          SNAP = 0;
      }
      break;
    case 4:
      SendCC(CHAN, cc_taptempo, 100);
      break;
  
    }
    break;
    
   case 2: //select snapshots
    switch (pin) {
     case 1: 
       SendCC(CHAN, cc_snapshot, 0);
       break;
    case 2: 
       SendCC(CHAN, cc_snapshot, 1);
       break;
    case 3: 
       SendCC(CHAN, cc_snapshot, 2);
       break;
    case 4:
       SendCC(CHAN, cc_snapshot, 3);
       break;      
    }
    break;    
    
   case 3: 
    switch (pin) {
     case 1: 
       if (fs7 == 0) {
           SendCC(CHAN, cc_fs7, 127);
           fs7 = 1;
       } else {
           SendCC(CHAN, cc_fs7, 0);
           fs7 = 0;                  
       }
       break;
    case 2: 
       if (fs8 == 0) {
           SendCC(CHAN, cc_fs8, 127);
           fs8 = 1;
       } else {
           SendCC(CHAN, cc_fs8, 0);
           fs8 = 0;                  
       }
       break;
    case 3: 
       if (fs9 == 0) {
           SendCC(CHAN, cc_fs9, 127);
           fs9 = 1;
       } else {
           SendCC(CHAN, cc_fs9, 0);
           fs9 = 0;                  
       }
       break;
    case 4:
       if (fs10 == 0) {
           SendCC(CHAN, cc_fs10, 127);
           fs10 = 1;
       } else {
           SendCC(CHAN, cc_fs10, 0);
           fs10 = 0;                  
       }
       break;      
    }
    break;
  }
}

void doubleClickEvent(int pin) {
// What happens: bank up
  ledVal2 = !ledVal2;
  digitalWrite(ledPin2, ledVal2);
}

void holdEvent(int pin) {
// What happens: Switch snapshot
  ledVal3 = !ledVal3;
  digitalWrite(ledPin3, ledVal3);  
    switch (pin) {
     case 1:  // mode 1 
          digitalWrite(modePin1, 1);
          digitalWrite(modePin2, 0);
          digitalWrite(modePin3, 0);
          MODE = 1;          
      break;
    case 2: 
          digitalWrite(modePin1, 0);
          digitalWrite(modePin2, 1);
          digitalWrite(modePin3, 0);          
          MODE = 2;          
      break;
    case 3:
          digitalWrite(modePin1, 0);
          digitalWrite(modePin2, 0);
          digitalWrite(modePin3, 1);          
          MODE = 3;          
      break;
    case 4:
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
  digitalWrite(modePin1, 1);
  digitalWrite(modePin2, 0);
  digitalWrite(modePin3, 0);    
// midi config
  MIDI.begin(MIDI_CHANNEL_OMNI);
}

void loop()
{
// Get button event and act accordingly
  for (int i = 1; i <= 5; i++) {
     int b = checkButton(i);
     if (b == 1) clickEvent(i);
     if (b == 2) doubleClickEvent(i);
     if (b == 3) holdEvent(i);
     if (b == 4) longHoldEvent(i); 
    } 
}
