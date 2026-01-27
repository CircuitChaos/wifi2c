#include <EEPROM.h>
#include <string.h>
#include <coredecls.h>
#include "storage.h"
#include "config.h"

static const uint32_t magic = 0xdeadbeef;

struct Data {
	struct Header {
		uint32_t magic;
		uint32_t crc32;
	} __attribute__((packed)) header;

	struct Payload {
		char ssid[config::SSID_SIZE];
		char psk[config::PSK_SIZE];
	} __attribute__((packed)) payload;
} __attribute__((packed));

static bool isSane(const char *ssid, const char *psk)
{
	return memchr(ssid, 0, config::SSID_SIZE) && memchr(psk, 0, config::PSK_SIZE);
}

bool storage::load(char *ssid, char *psk)
{
	Data data;
	EEPROM.begin(sizeof(data));
	EEPROM.get(0, data);
	EEPROM.end();

	if(data.header.magic != magic) {
		return false;
	}

	if(crc32(&data.payload, sizeof(data.payload)) != data.header.crc32) {
		return false;
	}

	if(!isSane(data.payload.ssid, data.payload.psk)) {
		return false;
	}

	memcpy(ssid, data.payload.ssid, config::SSID_SIZE);
	memcpy(psk, data.payload.psk, config::PSK_SIZE);
	return true;
}

bool storage::save(const char *ssid, const char *psk)
{
	if(!isSane(ssid, psk)) {
		return false;
	}

	Data data;
	memset(&data.payload, 0, sizeof(data.payload));
	strncpy(data.payload.ssid, ssid, config::SSID_SIZE - 1);
	strncpy(data.payload.psk, psk, config::PSK_SIZE - 1);

	data.header.magic = magic;
	data.header.crc32 = crc32(&data.payload, sizeof(data.payload));

	EEPROM.begin(sizeof(data));
	EEPROM.put(0, data);
	return EEPROM.end();
}
