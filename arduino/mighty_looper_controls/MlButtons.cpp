#include "MlButtons.h"

MlSimpleButton::MlSimpleButton(uint8_t pin, String baseSignal): pin(pin), baseSignal(baseSignal) {
/*
	this->pin = pin;
	this->baseSignal = baseSignal;
	this->buttonStates = 0;
*/
	this->buttonStates = 0; 
	pinMode(pin, INPUT_PULLUP);
}

bool MlSimpleButton::update(void) {
	// read the state of the switch into a local variable:
	//uint8_t reading = digitalRead(pin);
	bool reading = digitalRead(pin);

	// check to see if you just pressed the button
	// (i.e. the input went from LOW to HIGH),  and you've waited
	// long enough since the last press to ignore any noise:
	
	// If the switch changed, due to noise or pressing:
	if (reading != bitRead(buttonStates, LAST)) { 
		// reset the debouncing timer
		lastDebounceTime = millis();
	}

	if((millis() - lastDebounceTime) > debounceDelay) {
		// whatever the reading is at, it's been there for longer
		// than the debounce delay, so take it as the actual current state:
		updateButtonState(reading);
	}

	// save the reading.  Next time through the loop,
	// it'll be the lastButtonState:
	bitWrite(buttonStates, LAST, reading);
	return bitRead(buttonStates, NEW_PRESS);
}

void MlSimpleButton::updateButtonState(bool updatedState) {
	if(updatedState != bitRead(buttonStates, CURRENT)) { 
		// button state changed
		bitWrite(buttonStates, CURRENT, updatedState);
		if(updatedState == HIGH) {
			// button pressed
			bitWrite(buttonStates, NEW_PRESS, 1);
		}
	}
}

String MlSimpleButton::getSignal(void) {
	bitWrite(buttonStates, NEW_PRESS, 0);
	return "[" + baseSignal + "]";
}

MlMultiButton::MlMultiButton(uint8_t pin, String baseSignal): MlSimpleButton(pin, baseSignal) {
	this->pressCounter = 0;
	this->pressedTime = -1;
}

void MlMultiButton::updateButtonState(bool updatedState) {
	if(pressCounter == 1 && millis() - releasedTime > doublePressGap) {
		// too much time has passed since last button press -> double press won't be generated -> reset counter
		pressCounter = 0;
	}
	if(updatedState != bitRead(buttonStates, CURRENT)) { 
		//button state changed
		bitWrite(buttonStates, CURRENT, updatedState);
		if(updatedState == HIGH) {
			// button pressed
			if(pressCounter == 0) {
				pressedTime = millis();
			}
			if(pressCounter == 1) {
				// double press
				bitWrite(buttonStates, NEW_PRESS, 1);
				event = DOUBLE_PRESS;
				pressCounter = 0;
				//pressedTime = -1;
			}
		} else {
			// button released
			if(pressCounter == 0 & pressedTime != -1 & millis() - pressedTime < longPressSpan) {
				// single press
				bitWrite(buttonStates, NEW_PRESS, 1);
				event = SINGLE_PRESS;
				pressCounter++;
				pressedTime = -1;
				releasedTime = millis();
			}
		}
	} else {
		// button state remains
		if(updatedState == HIGH & pressedTime != -1 & millis() - pressedTime >= longPressSpan) {
			// button is pressed long enough to generate long press event
			bitWrite(buttonStates, NEW_PRESS, 1);
			event = LONG_PRESS;
			pressedTime = -1;
		}				
	}
}

String MlMultiButton::getSignal(void) {
	bitWrite(buttonStates, NEW_PRESS, 0);
	return "[" + baseSignal + "_" + event + "]";
}


MlChannelButton::MlChannelButton(uint8_t pin, String baseSignal, String channel): MlMultiButton(pin, baseSignal), channel(channel) {
	// nothing to do here
}

String MlChannelButton::getSignal(void) {
	bitWrite(buttonStates, NEW_PRESS, 0);
	return "[" + baseSignal + "_" + event + "_ch" + " " + channel + "]";
}

MlTrackButton::MlTrackButton(uint8_t pin, String baseSignal, String channel, String track): MlChannelButton(pin, baseSignal, channel), track(track) {
	// nothing to do here
}

String MlTrackButton::getSignal(void) {
	bitWrite(buttonStates, NEW_PRESS, 0);
	return "[" + baseSignal + "_" + event + "_t" + " " + channel + " " + track + "]";
}
