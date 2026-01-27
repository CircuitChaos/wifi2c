#include <Arduino.h>
#include "blinker.h"
#include "config.h"
#include "led.h"

Blinker::Blinker()
{
	ledSet(false);
}

Blinker::~Blinker()
{
	ledSet(false);
}

void Blinker::setMode(bool fast)
{
	isFast     = fast;
	nextMillis = 0;
}

void Blinker::loop()
{
	const unsigned long curMillis(millis());
	if(curMillis < nextMillis) {
		return;
	}

	nextMillis = curMillis + (isFast ? config::FAST_BLINK_MS : config::SLOW_BLINK_MS);
	curState   = !curState;
	ledSet(curState);
}
