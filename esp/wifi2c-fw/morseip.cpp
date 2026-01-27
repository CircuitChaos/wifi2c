#include <cstring>
#include "morseip.h"
#include "config.h"
#include "led.h"

static const size_t MORSE_BUF_SIZE = 8;

MorseIP::MorseIP()
{
	evtList.clear();
	ledSet(false);
}

MorseIP::~MorseIP()
{
	stop();
}

void MorseIP::start(IPAddress addr)
{
	char text[20];
	snprintf(text, sizeof(text), "%u.%u.%u.%u = ", addr[0], addr[1], addr[2], addr[3]);
	evtList.clear();
	ledSet(false);

	for(const char *p(text); *p; ++p) {
		addChar(*p);
	}

	nextMillis = millis();
}

void MorseIP::stop()
{
	evtList.clear();
	ledSet(false);
	nextMillis = 0;
}

void MorseIP::loop()
{
	if(!nextMillis) {
		return;
	}

	const unsigned long curMillis(millis());
	if(curMillis < nextMillis) {
		return;
	}

	uint8_t ticks;
	bool keydown;
	if(!evtList.get(ticks, keydown)) {
		stop();
		return;
	}

	ledSet(keydown);
	nextMillis = curMillis + (unsigned long) ticks * 1000 / (config::CW_WPM * 50. / 60);
}

void MorseIP::addChar(char ch)
{
	if(ch == ' ') {
		/* inter-word space: keyup for 4x dottime (as 3x was from a previous inter-character space) */
		evtList.add(4, false);
		return;
	}

	char buf[MORSE_BUF_SIZE];
	charToMorse(buf, ch);

	for(const char *p(buf); *p; ++p) {
		/* dash: keydown for 3x dottime, keyup for 1x dottime */
		/* dot: keydown for 1x dottime, keyup for 1x dottime */
		evtList.add((*p == '-') ? 3 : 1, true);
		evtList.add(1, false);
	}

	/* inter-character space: keyup for 2x dottime (as 1x was from previous symbol) */
	evtList.add(2, false);
}

void MorseIP::charToMorse(char *morseBuf, char ch)
{
	// TODO move to program space
	struct {
		char ch;
		const char *morse;
	} static const map[] = {
	    {'a', ".-"},
	    {'b', "-..."},
	    {'c', "-.-."},
	    {'d', "-.."},
	    {'e', "."},
	    {'f', "..-."},
	    {'g', "--."},
	    {'h', "...."},
	    {'i', ".."},
	    {'j', ".---"},
	    {'k', "-.-"},
	    {'l', ".-.."},
	    {'m', "--"},
	    {'n', "-."},
	    {'o', "---"},
	    {'p', ".--."},
	    {'q', "--.-"},
	    {'r', ".-."},
	    {'s', "..."},
	    {'t', "-"},
	    {'u', "..-"},
	    {'v', "...-"},
	    {'w', ".--"},
	    {'x', "-..-"},
	    {'y', "-.--"},
	    {'z', "--.."},
	    {'1', ".----"},
	    {'2', "..---"},
	    {'3', "...--"},
	    {'4', "....-"},
	    {'5', "....."},
	    {'6', "-...."},
	    {'7', "--..."},
	    {'8', "---.."},
	    {'9', "----."},
	    {'0', "-----"},
	    {',', "..-.."},
	    {'.', ".-.-.-"},
	    {'?', "..--.."},
	    {';', "-.-.-"},
	    {':', "---..."},
	    {'/', "-..-."},
	    {'+', ".-.-."},
	    {'-', "-....-"},
	    {'=', "-...-"},
	};

	for(size_t i(0); i < sizeof(map) / sizeof(*map); ++i) {
		if(map[i].ch == ch) {
			strncpy(morseBuf, map[i].morse, MORSE_BUF_SIZE - 1);
			break;
		}
	}

	morseBuf[MORSE_BUF_SIZE - 1] = 0;
}
