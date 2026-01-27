#include "wificonfig.h"
#include "twiserver.h"
#include "config.h"
#include "led.h"

static WiFiConfig *wificonfig;
static TwiServer *twiserver;

void setup()
{
	ledInit();
	wificonfig = new WiFiConfig();
}

void loop()
{
	if(!twiserver) {
		if(wificonfig->loop()) {
			/* Done this way, because wificonfig's dtor deletes serial port
			 * that's on the same pins as TWI */
			char ssid[config::SSID_SIZE], psk[config::PSK_SIZE];
			wificonfig->get(ssid, psk);
			delete wificonfig;
			twiserver = new TwiServer(ssid, psk);
		}
	}
	else {
		twiserver->loop();
	}
}
