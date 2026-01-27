#pragma once

#include <cstdint>

class Twi {
public:
	Twi(uint8_t scl, uint8_t sda, uint8_t defClk);
	~Twi();

	void setClk(uint8_t clk);
	void reset();
	void start();
	void stop();
	uint8_t read(bool ack);
	bool write(uint8_t byte);

private:
	uint8_t scl;
	uint8_t sda;
	uint8_t clk;

	bool readBit();
	void writeBit(bool bit);

	void delay();

	void sclhi();
	void scllo();
	// TODO add clock stretching
	// bool sclin();

	void sdahi();
	void sdalo();
	bool sdain();
};
