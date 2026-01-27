#include "morseevt.h"

bool MorseEvtList::get(uint8_t &ticks, bool &keydown)
{
	if(tail == 0) {
		/* Buffer is empty, nothing to return */
		return false;
	}

	ticks   = buf[head] & 0x7f;
	keydown = buf[head] & 0x80;

	if(++head == tail) {
		head = 0;
	}

	return true;
}

void MorseEvtList::add(uint8_t ticks, bool keydown)
{
	if(buf.size() <= tail) {
		/* +10 to avoid constant resizing */
		buf.resize(tail + 10);
	}

	buf[tail++] = ticks | (keydown ? 0x80 : 0);
}

void MorseEvtList::clear()
{
	head = tail = 0;
}
