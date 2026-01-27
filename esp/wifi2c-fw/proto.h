#pragma once

#include <cinttypes>
#include <cstddef>

namespace proto {

enum Event {
	EVT_NONE,
	EVT_KEEPALIVE,
	EVT_CLK,
	EVT_RESET,
	EVT_START,
	EVT_STOP,
	EVT_RD_ACK,
	EVT_RD_NAK,
	EVT_WR,
};

/* Requests:
 * - k: keepalive
 * - cxx: set bit clock (xx = hex, us * 10)
 * - e: reset bus
 * - s: start
 * - p: stop
 * - ra: read and send ACK
 * - rn: read and send NAK
 * - wxx: write (xx = hex)
 *
 * Recognized commands are echoed back. In case of error, no echo
 * is sent.
 *
 * Everything here (hex digits too) is lower-case.
 *
 * Responses:
 * - ra, rn: responds with xx (hex byte)
 * - wxx: responds with 'a' or 'n'
 */

class Parser {
public:
	void reset();

	/* data is used only if EVT_WR or EVT_CLK are returned */
	/* doEcho is true if input byte should be echoed back */
	Event operator()(uint8_t in, uint8_t &data, bool &doEcho);

private:
	enum State {
		ST_IDLE,
		ST_RD_ACK, /* 'r' received, reading 'A' or 'N' */
		ST_WR_HI,  /* 'w' received, reading first nibble */
		ST_WR_LO,  /* 'w' received, reading second nibble */
		ST_CLK_HI, /* 'c' received, reading first nibble */
		ST_CLK_LO, /* 'c' received, reading second nibble */
	};

	State state{ST_IDLE};
	uint8_t hi;
};

void generateReadResponse(uint8_t outBuf[2], uint8_t byte);
uint8_t generateWriteResponse(bool ack);

} // namespace proto
