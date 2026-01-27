#pragma once

#include <cinttypes>

namespace config {

static const uint8_t PIN_SCL = 3;
static const uint8_t PIN_SDA = 1;
static const uint8_t PIN_LED = 2;

static const uint16_t TCP_PORT    = 2430;
static const uint32_t I2C_DEF_CLK = 0xff;

static const unsigned long SERIAL_BAUD       = 115200;
static const unsigned long CONFIG_TIMEOUT_MS = 5000;

/* Including NUL, so value - 1 */
static const uint8_t SSID_SIZE = 33;
static const uint8_t PSK_SIZE  = 65;

static const unsigned long SLOW_BLINK_MS = 500;
static const unsigned long FAST_BLINK_MS = 100;
static const uint8_t CW_WPM              = 5;

} // namespace config
