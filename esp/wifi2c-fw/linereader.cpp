#include "linereader.h"

LineReader::LineReader(char *buf, uint8_t bufsz) : buf(buf), bufsz(bufsz)
{
}

bool LineReader::operator()(char ch, bool &doEcho)
{
	doEcho = false;

	if(ch == 0x0a) {
		return false;
	}

	if(ch == 0x0d) {
		buf[idx] = 0;
		doEcho   = true;
		return true;
	}

	if(ch == 0x08) {
		if(idx) {
			doEcho = true;
			--idx;
		}
		return false;
	}

	if(idx < (bufsz - 1) && ch >= 0x20) {
		doEcho     = true;
		buf[idx++] = ch;
	}

	return false;
}
