#pragma once

#include <IPAddress.h>
#include "morseevt.h"

class MorseIP {
public:
	MorseIP();
	~MorseIP();

	void start(IPAddress addr);
	void stop();
	void loop();

private:
	unsigned long nextMillis{0};
	MorseEvtList evtList;

	void addChar(char ch);
	void charToMorse(char *morseBuf, char ch);
};
