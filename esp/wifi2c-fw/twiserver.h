#pragma once

#include <cinttypes>
#include <ESP8266WiFi.h>
#include "morseip.h"
#include "config.h"
#include "proto.h"
#include "twi.h"

class TwiServer {
public:
	TwiServer(const char *ssid, const char *psk);
	void loop();

private:
	WiFiServer wifiServer{config::TCP_PORT};
	WiFiClient wifiClient;
	proto::Parser parser;
	Twi twi{config::PIN_SCL, config::PIN_SDA, config::I2C_DEF_CLK};
	bool haveWiFi{false};
	MorseIP morseip;

	void sendHello();
	void handleReadByte(uint8_t byte);
};
