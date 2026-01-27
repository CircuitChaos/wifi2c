#pragma once

#include <cinttypes>

class LineReader {
public:
	LineReader(char *buf, uint8_t busz);
	bool operator()(char ch, bool &doEcho);

private:
	char *const buf;
	const uint8_t bufsz;
	uint8_t idx{0};
};
