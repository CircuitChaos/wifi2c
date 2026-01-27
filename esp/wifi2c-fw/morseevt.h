#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>

class MorseEvtList {
public:
	bool get(uint8_t &ticks, bool &keydown);
	void add(uint8_t ticks, bool keydown);
	void clear();

private:
	/* To save RAM, MorseEvt is kept as a single byte:
	 * - b8: keydown
	 * - b7...b0: ticks
	 *
	 * To avoid RAM fragmentation, we keep a buffer here
	 * that grows, but never shrinks.
	 *
	 * If vector is too memory intensive, I'll switch to realloc().
	 */
	std::vector<uint8_t> buf;
	size_t head{0}; /* Currently read position */
	size_t tail{0}; /* Currently free position to which add() adds, advanced after each add() */
};
