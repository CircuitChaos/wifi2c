#include <Arduino.h>
#include "led.h"
#include "config.h"

static bool ledState = false;

void ledInit()
{
	pinMode(config::PIN_LED, OUTPUT);
	ledSet(false);
	ledState = false;
}

void ledSet(bool on)
{
	if(ledState == on) {
		return;
	}

	digitalWrite(config::PIN_LED, on ? LOW : HIGH);
	ledState = on;
}
