/* If WiFi config is already set:
 * - starts serial port (RX only)
 * - waits CONFIG_TIMEOUT_MS for ENTER on serial port
 * - if ENTER is pressed, starts configurator
 * - if ENTER is NOT pressed, returns true from loop() and returns settings
 *
 * If WiFi config is not set, starts configurator.
 *
 * Configurator:
 * - starts serial port (duplex)
 * - starts configurator
 * - once WiFi config is set, saves it and asks to reboot
 */

#pragma once

#include <cinttypes>
#include "config.h"
#include "linereader.h"
#include "storage.h"
#include "blinker.h"

class WiFiConfig {
public:
	WiFiConfig();
	~WiFiConfig();

	/* If true, can read WiFi config */
	bool loop();

	void get(char *ssid_, char *psk_);

private:
	char ssid[config::SSID_SIZE];
	char psk[config::PSK_SIZE];
	const bool haveConfig;

	enum State {
		ST_WAIT_ENTER,
		ST_ENTER_SSID,
		ST_ENTER_PSK,
		ST_REBOOT,
	};

	State state{ST_WAIT_ENTER};
	unsigned long startMillis;
	LineReader *lineReader;
	Blinker blinker;
};
