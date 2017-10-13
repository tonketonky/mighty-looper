/*
 * Mighty Looper
 * 
 * This is a sketch for Arduino part of the project "Mighty Looper".
 * Another part is Pure Data patch mighty_looper.pd running on 
 * Raspberry Pi 3 which implements core functionalities and features 
 * of Mighty Looper.
 *    
 * This code provides the following:
 *    - handling inputs from 8 momentary switches (one for each track) 
 *      that can produce 3 different signals:
 *          - single press  - signal for toggling playback ON and OFF
 *                          - also signal for starting and stopping 
 *                            recording of the first track of a given 
 *                            verse in "Free length" recording mode 
 *          - double press  - signal for starting and stopping recording
 *          - long press    - signal for UNDO/REDO last recorded layer 
 *    - handling inputs from 13 potentiometers:
 *        - 8 pots for adjusting track volumes (one for each)
 *        - 2 pots for adjusting input channel volumes (one for each)
 *        - 1 pot for adjusting click volume in "Fixed length" recording mode
 *        - 2 pots for adjusting overall output channel volumes (one for each)
 *    - handling inputs of 5 lever switches:
 *        - 1 for switching between "Free length" and "Fixed length" recording mode
 *        - 1 for switching between "STEREO" and "MONO" output mode
 *        - 1 for turning click ON and OFF in "Fixed length" recording mode
 *        - 2 for togging channel playback while recording ON and OFF (one for each)
 *    - displaying loop progress for each track playing back and flags for 
 *      tracks marked for recording/playback ON or OFF/UNDO or REDO 8-digit
 *      7-segment display (MAX7219)
 *    - communicating with PD patch migthy_looper.pd via serial port
 *    
 * 
 */

// button pins
const byte trackButtonPins[8] = {2, 3, 4, 5, 6, 7, 8, 9};

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

byte buttonStates = 0b11111111;
byte lastButtonStates = 0b11111111;

byte trackButtonPressedCounters[8] = {0, 0, 0 ,0 ,0 ,0 ,0, 0};
unsigned long trackButtonPressedTimes[8] = {0, 0, 0, 0, 0, 0, 0, 0};

unsigned long trackButtonDoublePressDelay = 500;
unsigned long trackButtonLongPressTime = 500;

byte tracksLooping = 0b00000000;
byte tracksRecording = 0b00000000;
byte tracksMarkedForLoop = 0b00000000; 
byte tracksMarkedForRec = 0b00000000; 
byte tracksMarkedForUndo = 0b00000000; 

unsigned long lastDebounceTimes[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned long debounceDelay = 10;

void setup() {
  for(byte buttonId = 0; buttonId < 8; buttonId++) {
    pinMode(trackButtonPins[buttonId], INPUT);
    digitalWrite(trackButtonPins[buttonId], HIGH);
  }
  
  pinMode(MAX7219_DIN, OUTPUT);   // serial data-in
  pinMode(MAX7219_CS, OUTPUT);    // chip-select, active low    
  pinMode(MAX7219_CLK, OUTPUT);   // serial clock
  digitalWrite(MAX7219_CS, HIGH);

  beatDuration = 60000 / tempo;
  loopPhaseDuration = beatDuration * beat[0] / 6;
  
  Serial.begin(9600);

  Serial.print("loop beat duration: ");
  Serial.println(beatDuration);

  Serial.print("loop phase duration: ");
  Serial.println(loopPhaseDuration);
  
  resetMAX7219();
}

void loop() {
  
  for(byte buttonId = 0; buttonId < 8; buttonId++) {
    // temporarily until all buttons are connected
    if(buttonId != 3 && buttonId !=7) {
      readTrackButton(buttonId);
    }
  }

  updateLoopProgress();
  updateMAX7219(); 
}

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

byte readTrackButton(byte buttonId) {
  if(trackButtonPressedCounters[buttonId] == 1 & millis() - trackButtonPressedTimes[buttonId] >= trackButtonDoublePressDelay) {
    trackButtonPressedCounters[buttonId] = 0;
  }
  // read the state of the switch into a local variable:
  byte reading = digitalRead(trackButtonPins[buttonId]);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != bitRead(lastButtonStates, buttonId)) {
    // reset the debouncing timer
    lastDebounceTimes[buttonId] = millis();
  }

  if((millis() - lastDebounceTimes[buttonId]) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if(reading != bitRead(buttonStates, buttonId)) {
      bitWrite(buttonStates, buttonId, reading); 
      
      // only handle button pressed event if the new button state is LOW
      if(bitRead(buttonStates, buttonId) == LOW) {
        trackButtonPressed(buttonId);  
      } else {
        trackButtonReleased(buttonId);
      }
    }
  }
  
  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  bitWrite(lastButtonStates, buttonId, reading);
}

void trackButtonPressed(byte buttonId) {
  Serial.println("button pressed");
  if(trackButtonPressedCounters[buttonId] == 0) {
    trackButtonPressedTimes[buttonId] = millis();
  }
}

void trackButtonReleased(byte buttonId) {
  Serial.println("button released");
  if(trackButtonPressedCounters[buttonId] == 0 & millis() - trackButtonPressedTimes[buttonId] > trackButtonLongPressTime) {
    handleTrackButtonLongPressed(buttonId);
  } else {
    switch(trackButtonPressedCounters[buttonId]) {
      case 0:
        handleTrackButtonSinglePress(buttonId);
        trackButtonPressedCounters[buttonId]++;
        break;
      case 1:
        if(millis() - trackButtonPressedTimes[buttonId] < trackButtonDoublePressDelay) {
          trackButtonPressedCounters[buttonId] = 0;
          undoTrackButtonSinglePress(buttonId);
          handleTrackButtonDoublePressed(buttonId);
        }
        break;
      default:
        break;
     }
  }
}

void handleTrackButtonSinglePress(byte buttonId) {
  Serial.println("single press");
  if(looping == 0) {
    startLooping();
    bitWrite(tracksLooping, buttonId, bitRead(tracksLooping, buttonId) ^ 1); // inverse bit
  } else {
    markUnmarkTrackForLoop(buttonId);
  }
}

void undoTrackButtonSinglePress(byte buttonId) {
  Serial.println("single press undo");
  markUnmarkTrackForLoop(buttonId);
}

void handleTrackButtonDoublePressed(byte buttonId) {
  Serial.println("double press");
  markUnmarkTrackForRec(buttonId);
}

void handleTrackButtonLongPressed(byte buttonId) {
  Serial.println("long press");
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
