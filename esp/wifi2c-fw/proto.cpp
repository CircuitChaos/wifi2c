#include "proto.h"

static bool fromHex(uint8_t &out, uint8_t in)
{
	if(in >= '0' && in <= '9') {
		out = in - '0';
		return true;
	}

	if(in >= 'a' && in <= 'f') {
		out = in - 'a' + 10;
		return true;
	}

	return false;
}

static uint8_t toHex(uint8_t in)
{
	return (in < 10) ? (in + '0') : (in - 10 + 'a');
}

void proto::Parser::reset()
{
	state = ST_IDLE;
}

proto::Event proto::Parser::operator()(uint8_t in, uint8_t &data, bool &doEcho)
{
	doEcho = false;

	switch(state) {
		case ST_IDLE:
			switch(in) {
				case 'k':
					doEcho = true;
					return EVT_KEEPALIVE;

				case 'c':
					doEcho = true;
					state  = ST_CLK_HI;
					return EVT_NONE;

				case 'e':
					doEcho = true;
					return EVT_RESET;

				case 's':
					doEcho = true;
					return EVT_START;

				case 'p':
					doEcho = true;
					return EVT_STOP;

				case 'r':
					doEcho = true;
					state  = ST_RD_ACK;
					return EVT_NONE;

				case 'w':
					doEcho = true;
					state  = ST_WR_HI;
					return EVT_NONE;

				default:
					break;
			}

			break;

		case ST_RD_ACK:
			state = ST_IDLE;

			if(in == 'a') {
				doEcho = true;
				return EVT_RD_ACK;
			}

			if(in == 'n') {
				doEcho = true;
				return EVT_RD_NAK;
			}

			break;

		case ST_WR_HI:
		case ST_CLK_HI:
			if(!fromHex(hi, in)) {
				state = ST_IDLE;
				break;
			}

			doEcho = true;
			state  = (state == ST_WR_HI) ? ST_WR_LO : ST_CLK_LO;
			break;

		case ST_WR_LO:
		case ST_CLK_LO: {
			const Event evt((state == ST_WR_LO) ? EVT_WR : EVT_CLK);
			state = ST_IDLE;

			uint8_t lo;
			if(!fromHex(lo, in)) {
				break;
			}

			doEcho = true;
			data   = (hi << 4) | lo;
			return evt;
		}

		default:
			break;
	}

	return EVT_NONE;
}

void proto::generateReadResponse(uint8_t outBuf[2], uint8_t byte)
{
	outBuf[0] = toHex(byte >> 4);
	outBuf[1] = toHex(byte & 0x0f);
}

uint8_t proto::generateWriteResponse(bool ack)
{
	return ack ? 'a' : 'n';
}
