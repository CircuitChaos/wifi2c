#pragma once

#include "config.h"

namespace storage {

bool load(char *ssid, char *psk);
bool save(const char *ssid, const char *psk);

} // namespace storage
