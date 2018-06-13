#define SINGLE_PRESS "sp" 
#define DOUBLE_PRESS "dp"
#define LONG_PRESS "lp"

#define CURRENT 0
#define LAST 1
#define NEW_PRESS 2

#include "Arduino.h"

class MlSimpleButton
{
	public:
		MlSimpleButton(uint8_t pin, String baseSignal);
		bool update(void);
		virtual String getSignal(void);
	private:
		uint8_t pin;
		unsigned long lastDebounceTime;
		static const uint8_t debounceDelay = 10;
		virtual void updateButtonState(bool updatedState);
	protected:
		uint8_t buttonStates;
		String baseSignal;
};

class MlMultiButton: public MlSimpleButton {
	public:
		MlMultiButton(uint8_t pin, String baseSignal);
		virtual String getSignal(void);
	private:
		uint8_t pressCounter;
		unsigned long pressedTime;
		unsigned long releasedTime;
		static const uint16_t doublePressGap = 500;
		static const uint16_t longPressSpan = 500;
		void updateButtonState(bool updatedState);
		String getEventCommand(uint8_t event);
	protected:
		String event;
};

class MlChannelButton: public MlMultiButton {
	public:
		MlChannelButton(uint8_t pin, String baseSignal, String channel);
		virtual String getSignal(void);
	protected:
		String channel;
};

class MlTrackButton: public MlChannelButton {
	public:
		MlTrackButton(uint8_t pin, String baseSignal, String channel, String track);
		String getSignal(void);
	private:
		String track;
};

