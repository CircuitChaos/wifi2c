#include <cstring>
#include <Arduino.h>
#include <HardwareSerial.h>
#include "wificonfig.h"

WiFiConfig::WiFiConfig() : startMillis(millis()), haveConfig(storage::load(ssid, psk))
{
	Serial.begin(config::SERIAL_BAUD, SERIAL_8N1, SERIAL_RX_ONLY);
}

WiFiConfig::~WiFiConfig()
{
	Serial.flush();
	Serial.end();
}

bool WiFiConfig::loop()
{
	blinker.loop();

	if(state == ST_WAIT_ENTER) {
		if(haveConfig && (millis() - startMillis) > config::CONFIG_TIMEOUT_MS) {
			return true;
		}

		if(Serial.read() == 0x0d) {
			blinker.setMode(false);
			Serial.end();
			Serial.begin(config::SERIAL_BAUD);
			Serial.write("\r\n");
			if(haveConfig) {
				Serial.printf("WiFi configuration found for SSID %s. Please provide new below.\r\n", ssid);
			}
			else {
				Serial.write("WiFi configuration not found. Please provide it below.\r\n");
			}
			Serial.write("Enter SSID: ");
			Serial.read(); /* Flush some strange char that's appearing */
			lineReader = new LineReader(ssid, config::SSID_SIZE);
			state      = ST_ENTER_SSID;
		}
	}
	else if(state == ST_ENTER_SSID || state == ST_ENTER_PSK) {
		const int ch(Serial.read());
		if(ch == -1) {
			return false;
		}

		bool doEcho;
		const bool done((*lineReader)(ch, doEcho));

		if(doEcho) {
			if(ch == 0x08) {
				Serial.write("\x08 ");
			}

			Serial.write(ch);
		}

		if(!done) {
			return false;
		}

		Serial.write(0x0a);

		if(state == ST_ENTER_SSID) {
			delete lineReader;
			Serial.write("Enter PSK: ");
			lineReader = new LineReader(psk, config::PSK_SIZE);
			state      = ST_ENTER_PSK;
		}
		else {
			delete lineReader;
			if(storage::save(ssid, psk)) {
				Serial.write("Please reboot the board to start I2C server.\r\n");
			}
			else {
				Serial.write("Error writing to flash! This shouldn't happen. Reboot and try again.\r\n");
			}
			state = ST_REBOOT;
		}
	}

	return false;
}

void WiFiConfig::get(char *ssid_, char *psk_)
{
	memcpy(ssid_, ssid, config::SSID_SIZE);
	memcpy(psk_, psk, config::PSK_SIZE);
}
