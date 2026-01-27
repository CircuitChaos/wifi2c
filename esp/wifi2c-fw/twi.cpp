#include "twi.h"
#include <Arduino.h>

Twi::Twi(uint8_t scl, uint8_t sda, uint8_t defClk) : scl(scl), sda(sda), clk(defClk)
{
	pinMode(scl, INPUT);
	pinMode(sda, INPUT);
	digitalWrite(scl, LOW);
	digitalWrite(sda, LOW);
	reset();
}

Twi::~Twi()
{
	stop();
}

void Twi::setClk(uint8_t clk_)
{
	clk = clk_;
}

/* See:
 * - https://www.analog.com/media/en/technical-documentation/application-notes/54305147357414an686_0.pdf
 * - https://renesasrulz.com/synergy/f/synergy---forum/10706/how-to-reset-i2c-bus
 * - https://www.microchip.com/forums/m175368.aspx
 */

void Twi::reset()
{
	sdahi();
	sclhi();

	for(uint8_t i(0); i < 9; ++i) {
		scllo();
		sclhi();
	}

	scllo();
	sdalo();
	sclhi();
	sdahi();
}

void Twi::start()
{
	/* These two are for repeated start */
	sdahi();
	sclhi();
	sdalo();
	scllo();
}

void Twi::stop()
{
	sdalo();
	sclhi();
	sdahi();
}

uint8_t Twi::read(bool ack)
{
	uint8_t rs(0);
	for(uint8_t i(0); i < 8; ++i) {
		if(readBit()) {
			rs |= 1 << (7 - i);
		}
	}

	writeBit(!ack);
	return rs;
}

bool Twi::write(uint8_t byte)
{
	for(uint8_t i(0); i < 8; ++i) {
		writeBit(byte & (1 << (7 - i)));
	}

	return !readBit();
}

bool Twi::readBit()
{
	sdahi();
	sclhi();
	const bool b(sdain());
	scllo();
	return b;
}

void Twi::writeBit(bool b)
{
	if(b) {
		sdahi();
	}
	else {
		sdalo();
	}

	sclhi();
	scllo();
}

void Twi::delay()
{
	if(clk) {
		delayMicroseconds((uint16_t) clk * 10);
	}
}

void Twi::sclhi()
{
	pinMode(scl, INPUT);
	delay();
}

void Twi::scllo()
{
	pinMode(scl, OUTPUT);
	delay();
}

void Twi::sdahi()
{
	pinMode(sda, INPUT);
	delay();
}

void Twi::sdalo()
{
	pinMode(sda, OUTPUT);
	delay();
}

bool Twi::sdain()
{
	return digitalRead(sda) == HIGH;
}
