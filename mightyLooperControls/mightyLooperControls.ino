/*
 * Mighty Looper
 * 
 * This is a sketch for Arduino part of the project "Mighty Looper".
 * Another part is Pure Data patch mighty_looper.pd running on 
 * Raspberry Pi 3 which implements core functionalities and features 
 * of Mighty Looper.
 *
 */

#include "mlButtons.h"

/* BKP OF CODE TO BE MODIFIED AND USED LATER - BEGIN 
// 8-digit 7-semgent display (MAX7219) pins 
const byte MAX7219_DIN  = 10;
const byte MAX7219_CS   = 11;
const byte MAX7219_CLK  = 12;

// enum of 8-digit 7-semgent display (MAX7219) register address codes
enum {  MAX7219_REG_DECODE    = 0x09,  
        MAX7219_REG_INTENSITY = 0x0A,
        MAX7219_REG_SCANLIMIT = 0x0B,
        MAX7219_REG_SHUTDOWN  = 0x0C,
        MAX7219_REG_DISPTEST  = 0x0F };

// enum of ON, OFF values
enum  { OFF = 0,  
        ON  = 1 };


byte loopPhases[6] = {
  0b00000100,
  0b00000110,
  0b01000110,
  0b01100110,
  0b01110110,
  0b01111110
};

byte loopPhase = 0;
unsigned long loopPhaseStartTime = 0;
unsigned long loopPhaseDuration = 0;

byte beatCount = 1;
unsigned long beatStartTime = 0;
unsigned long beatDuration = 0;

byte looping = 0;

byte tempo = 100;
byte beat[2] = {4, 4};
BKP OF CODE TO BE MODIFIED AND USED LATER - END */

MlTrackButton trackButtons[16] = {
  MlTrackButton(2, "r", "ch1", "t1"),
  MlTrackButton(3, "p", "ch1", "t1"),
  MlTrackButton(4, "r", "ch1", "t2"),
  MlTrackButton(5, "p", "ch1", "t2"),
  MlTrackButton(6, "r", "ch1", "t3"),
  MlTrackButton(7, "p", "ch1", "t3"),
  MlTrackButton(8, "r", "ch1", "t4"),
  MlTrackButton(9, "p", "ch1", "t4"),
  MlTrackButton(10, "r", "ch2", "t1"),
  MlTrackButton(11, "p", "ch2", "t1"),
  MlTrackButton(12, "r", "ch2", "t2"),
  MlTrackButton(13, "p", "ch2", "t2"),
  MlTrackButton(14, "r", "ch2", "t3"),
  MlTrackButton(15, "p", "ch2", "t3"),
  MlTrackButton(16, "r", "ch2", "t4"),
  MlTrackButton(17, "p", "ch2", "t4")
 };
/*
MlTrackButton button1(2, "r", "ch1", "t1");
MlTrackButton button2(3, "p", "ch1", "t1");

MlTrackButton button3(4, "r", "ch1", "t2");
MlTrackButton button4(5, "p", "ch1", "t2");

MlTrackButton button5(6, "r", "ch1", "t3");
MlTrackButton button6(7, "p", "ch1", "t3");
*/

/* BKP OF CODE TO BE MODIFIED AND USED LATER - BEGIN
byte tracksLooping = 0b00000000;
byte tracksRecording = 0b00000000;
byte tracksMarkedForLoop = 0b00000000; 
byte tracksMarkedForRec = 0b00000000; 
byte tracksMarkedForUndo = 0b00000000; 
BKP OF CODE TO BE MODIFIED AND USED LATER - END */

void setup() {
/* BKP OF CODE TO BE MODIFIED AND USED LATER - BEGIN
  pinMode(MAX7219_DIN, OUTPUT);   // serial data-in
  pinMode(MAX7219_CS, OUTPUT);    // chip-select, active low    
  pinMode(MAX7219_CLK, OUTPUT);   // serial clock
  digitalWrite(MAX7219_CS, HIGH);

  resetMAX7219();

  beatDuration = 60000 / tempo;
  loopPhaseDuration = beatDuration * beat[0] / 6;
*/
  Serial.begin(9600); 
}

void loop() {
  //BKP resetMAX7219();

  for(int id = 0; id < 16; id++) {
    if(trackButtons[id].update()) {
      Serial.print(trackButtons[id].getSignal());
    }
  }

  // BKP updateLoopProgress();
  // BKP updateMAX7219(); 
}

/* BKP OF CODE TO BE MODIFIED AND USED LATER - END
void updateLoopProgress() {
  if(looping == 1) {
    if(millis() - beatStartTime > beatDuration) {
      beatStartTime += beatDuration;
      if(beatCount < beat[0]) {
        beatCount++;
      } else {
        beatCount=1;
        tracksLooping = tracksLooping ^ tracksMarkedForLoop;
        tracksMarkedForLoop = 0b00000000;
      }  
    }
    
    if(millis() - loopPhaseStartTime > loopPhaseDuration) {
      loopPhaseStartTime += loopPhaseDuration;
      if(loopPhase < 5) {
        loopPhase++;
      } else {
        loopPhase = 0;
      }
    }
  }
}




void markUnmarkTrackForLoop(byte trackNum) {
  bitWrite(tracksMarkedForLoop, trackNum, bitRead(tracksMarkedForLoop, trackNum) ^ 1); // inverse bit 
}

void markUnmarkTrackForRec(byte trackNum) {
  bitWrite(tracksMarkedForRec, trackNum, bitRead(tracksMarkedForRec, trackNum) ^ 1); // inverse bit 
}

void markUnmarkTrackForUndo(byte trackNum) {
  bitWrite(tracksMarkedForUndo, trackNum, bitRead(tracksMarkedForUndo, trackNum) ^ 1); // inverse bit 
}

void startLooping() {
  loopPhaseStartTime = millis();
  beatStartTime = millis();
  looping = 1;
}

void updateMAX7219() {
  // iterate over all 8 tracks and set corresponging register value to display
  for(byte i = 0; i < 8; i++) {
    // set current loop phase to regValue variable if given track is looping, else leave empty byte
    int regValue = 0b00000000;
    if(bitRead(tracksLooping, i) == 1) {
      regValue = loopPhases[loopPhase]; 
    }
    
    // update marked flag bit in regVal according to corresponing bit of tracksMarkedForLoop variable
    regValue = bitWrite(regValue, 7, bitRead(regValue, 7) ^ bitRead(tracksMarkedForLoop, i));

    // set corresponding register for current track; 7-trackNum for displaying from left to right
    setRegister(7-i+1, regValue);
  }
}

void setRegister(byte reg, byte value) {
    digitalWrite(MAX7219_CS, LOW);
    shiftOut(MAX7219_DIN, MAX7219_CLK, MSBFIRST, reg);
    shiftOut(MAX7219_DIN, MAX7219_CLK, MSBFIRST, value);
    digitalWrite(MAX7219_CS, HIGH);
}

void resetMAX7219() {
  setRegister(MAX7219_REG_SHUTDOWN, OFF);       // turn off display
  setRegister(MAX7219_REG_DISPTEST, OFF);       // turn off test mode
  setRegister(MAX7219_REG_INTENSITY, 0x0F);     // display intensity
  setRegister(MAX7219_REG_SCANLIMIT, 7);        // limit to 8 digits
  setRegister(MAX7219_REG_DECODE, 0b00000000);  // non-decode mode for all digits
  setRegister(MAX7219_REG_SHUTDOWN, ON);
}

BKP OF CODE TO BE MODIFIED AND USED LATER - END */
