#pragma once

#include <inttypes.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WIFI2C_VERSION      1
#define WIFI2C_DEFAULT_PORT "2430"

typedef int wifi2c_hnd;
typedef int wifi2c_err;

enum {
	WIFI2C_ERR_OK = 0,          /* No error */
	WIFI2C_ERR_VERSION,         /* Unknown FW version, upgrade library */
	WIFI2C_ERR_PROTOCOL,        /* Protocol error */
	WIFI2C_ERR_TIMEOUT,         /* No response from the ESP */
	WIFI2C_ERR_INTERNAL,        /* Internal library inconsistency, please report it */
	WIFI2C_ERR_GAI_BASE = 1000, /* getaddrinfo() error base; note that you have to negate resulting error */
	WIFI2C_ERR_SYS_BASE = 2000, /* System error base */
};

int wifi2c_get_version(void);
const char *wifi2c_get_error_string(wifi2c_err err);

wifi2c_err wifi2c_open(wifi2c_hnd *hnd, const char *host, const char *port);
wifi2c_err wifi2c_close(wifi2c_hnd hnd);

wifi2c_err wifi2c_set_clock(wifi2c_hnd hnd, uint8_t clock);
wifi2c_err wifi2c_keepalive(wifi2c_hnd hnd);

wifi2c_err wifi2c_reset(wifi2c_hnd hnd);
wifi2c_err wifi2c_start(wifi2c_hnd hnd);
wifi2c_err wifi2c_stop(wifi2c_hnd hnd);
wifi2c_err wifi2c_write(wifi2c_hnd hnd, uint8_t byte, bool *ack);
wifi2c_err wifi2c_read(wifi2c_hnd hnd, uint8_t *byte, bool ack);

#ifdef __cplusplus
} // extern "C"
#endif
