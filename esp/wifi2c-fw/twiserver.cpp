#include "twiserver.h"

TwiServer::TwiServer(const char *ssid, const char *psk)
{
	WiFi.begin(ssid, psk);
	wifiServer.begin();
}

void TwiServer::loop()
{
	morseip.loop();

	if(WiFi.status() == WL_CONNECTED) {
		if(!haveWiFi) {
			morseip.start(WiFi.localIP());
			haveWiFi = true;
		}
	}
	else {
		if(haveWiFi) {
			morseip.stop();
			if(wifiClient) {
				wifiClient.stop();
			}
			haveWiFi = false;
		}
	}

	WiFiClient newClient(wifiServer.accept());
	if(newClient) {
		if(wifiClient) {
			wifiClient.stop();
		}

		wifiClient = newClient;
		sendHello();
		parser.reset();
	}

	if(wifiClient) {
		if(!wifiClient.connected()) {
			wifiClient.stop();
		}
		else {
			uint8_t byte;
			if(wifiClient.read(&byte, 1) == 1) {
				handleReadByte(byte);
			}
		}
	}
}

void TwiServer::sendHello()
{
	wifiClient.write("wifi2c v01");
}

void TwiServer::handleReadByte(uint8_t byte)
{
	uint8_t data;
	bool doEcho;

	const proto::Event evt(parser(byte, data, doEcho));
	if(doEcho) {
		wifiClient.write(byte);
	}

	switch(evt) {
		case proto::EVT_CLK:
			twi.setClk(data);
			break;

		case proto::EVT_RESET:
			twi.reset();
			break;

		case proto::EVT_START:
			twi.start();
			break;

		case proto::EVT_STOP:
			twi.stop();
			break;

		case proto::EVT_RD_ACK:
		case proto::EVT_RD_NAK: {
			const uint8_t readByte(twi.read(evt == proto::EVT_RD_ACK));
			uint8_t rsp[2];
			proto::generateReadResponse(rsp, readByte);
			wifiClient.write(rsp, 2);
			break;
		}

		case proto::EVT_WR:
			wifiClient.write(proto::generateWriteResponse(twi.write(data)));
			break;

		case proto::EVT_NONE:
		case proto::EVT_KEEPALIVE:
		default:
			break;
	}
}
