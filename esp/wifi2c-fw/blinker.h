#pragma once

class Blinker {
public:
	Blinker();
	~Blinker();

	void setMode(bool fast);
	void loop();

private:
	bool curState{false};
	bool isFast{true};
	unsigned long nextMillis{0};
};
