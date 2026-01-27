#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include "wifi2c.h"

static const unsigned TIMEOUT = 10000;

static bool from_hex(uint8_t *dst, uint8_t src)
{
	if(src >= '0' && src <= '9') {
		*dst = src - '0';
		return true;
	}

	if(src >= 'a' && src <= 'f') {
		*dst = src - 'a' + 10;
		return true;
	}

	return false;
}

static wifi2c_err get_or_put(wifi2c_hnd hnd, char *rbuf, const char *wbuf, size_t size)
{
	size_t pos = 0;

	while(pos < size) {
		struct pollfd pfd;
		int prs;
		ssize_t rwrs;

		pfd.fd      = hnd;
		pfd.events  = rbuf ? POLLIN : POLLOUT;
		pfd.revents = 0;

		prs = poll(&pfd, 1, TIMEOUT);
		if(prs < 0 && errno == EINTR) {
			continue;
		}

		if(prs <= 0) {
			return (prs == 0) ? WIFI2C_ERR_TIMEOUT : (WIFI2C_ERR_SYS_BASE + errno);
		}

		if(!(pfd.revents & (rbuf ? POLLIN : POLLOUT))) {
			return WIFI2C_ERR_INTERNAL;
		}

		if(rbuf) {
			rwrs = read(hnd, rbuf + pos, size - pos);
		}
		else {
			rwrs = write(hnd, wbuf + pos, size - pos);
		}

		if(rwrs < 0 && errno == EINTR) {
			continue;
		}

		if(rwrs <= 0) {
			/* In case of read, EOF means ERR_PROTOCOL
			 * In case of write, zero-return is forbidden is an internal error
			 */
			return (rwrs == 0) ? (rbuf ? WIFI2C_ERR_PROTOCOL : WIFI2C_ERR_INTERNAL) : (WIFI2C_ERR_SYS_BASE + errno);
		}

		if((size_t) rwrs > (size - pos)) {
			return WIFI2C_ERR_INTERNAL;
		}

		pos += (size_t) rwrs;
	}

	return WIFI2C_ERR_OK;
}

static wifi2c_err get(wifi2c_hnd hnd, char *buf, size_t size)
{
	return get_or_put(hnd, buf, NULL, size);
}

static wifi2c_err put(wifi2c_hnd hnd, const char *buf, size_t size)
{
	return get_or_put(hnd, NULL, buf, size);
}

/* Warning: at most 3 chars are accepted here */
static wifi2c_err xchg(wifi2c_hnd hnd, const char *data)
{
	size_t len;
	char echo[3];
	int err;

	len = strlen(data);
	if(len > 3) {
		return WIFI2C_ERR_INTERNAL;
	}

	err = put(hnd, data, len);
	if(err) {
		return err;
	}

	err = get(hnd, echo, len);
	if(err) {
		return err;
	}

	if(memcmp(data, echo, len) != 0) {
		return WIFI2C_ERR_PROTOCOL;
	}

	return WIFI2C_ERR_OK;
}

int wifi2c_get_version(void)
{
	return WIFI2C_VERSION;
}

const char *wifi2c_get_error_string(int err)
{
	if(err >= WIFI2C_ERR_SYS_BASE) {
		return strerror(err - WIFI2C_ERR_SYS_BASE);
	}

	if(err >= WIFI2C_ERR_GAI_BASE) {
		return gai_strerror(-(err - WIFI2C_ERR_GAI_BASE));
	}

	switch(err) {
		case WIFI2C_ERR_OK:
			return "No error";

		case WIFI2C_ERR_VERSION:
			return "Unknown FW version, upgrade library";

		case WIFI2C_ERR_PROTOCOL:
			return "Protocol error";

		case WIFI2C_ERR_TIMEOUT:
			return "No response from the ESP";

		case WIFI2C_ERR_INTERNAL:
			return "Internal library inconsistency, please report it";

		default:
			break;
	}

	return "Unknown error";
}

static wifi2c_err opentcp(int *fd, const char *host, const char *port)
{
	struct addrinfo hints, *ai, *p;
	int gaierr;
	int lasterrno = 0;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;

	gaierr = getaddrinfo(host, port ? port : WIFI2C_DEFAULT_PORT, &hints, &ai);
	if(gaierr != 0) {
		return (gaierr == EAI_SYSTEM) ? (WIFI2C_ERR_SYS_BASE + errno) : (WIFI2C_ERR_GAI_BASE - gaierr);
	}

	*fd = -1;
	for(p = ai; p; p = p->ai_next) {
		*fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(*fd < 0) {
			lasterrno = errno;
			continue;
		}

		if(!connect(*fd, p->ai_addr, p->ai_addrlen)) {
			break;
		}

		lasterrno = errno;
		close(*fd);
		*fd = -1;
	}

	freeaddrinfo(ai);
	return (*fd < 0) ? (WIFI2C_ERR_SYS_BASE + lasterrno) : WIFI2C_ERR_OK;
}

wifi2c_err wifi2c_open(wifi2c_hnd *hnd, const char *host, const char *port)
{
	static const char *prefix = "wifi2c v";
	char recvbuf[strlen(prefix) + 2];
	int xfd = -1; /* Initialized to satisfy -Wmaybe-uninitialized */
	int err;

	err = opentcp(&xfd, host, port);
	if(err) {
		return err;
	}

	err = get(xfd, recvbuf, sizeof(recvbuf));
	if(err) {
		close(xfd);
		return err;
	}

	if(memcmp(recvbuf, prefix, strlen(prefix))) {
		close(xfd);
		return WIFI2C_ERR_PROTOCOL;
	}

	if(memcmp(recvbuf + strlen(prefix), "01", 2)) {
		close(xfd);
		return WIFI2C_ERR_VERSION;
	}

	if(hnd) {
		*hnd = xfd;
	}

	return WIFI2C_ERR_OK;
}

wifi2c_err wifi2c_close(wifi2c_hnd hnd)
{
	if(close(hnd) != 0) {
		return WIFI2C_ERR_SYS_BASE + errno;
	}

	return WIFI2C_ERR_OK;
}

wifi2c_err wifi2c_set_clock(wifi2c_hnd hnd, uint8_t clock)
{
	char buf[4];
	snprintf(buf, sizeof(buf), "c%02x", clock);
	return xchg(hnd, buf);
}

wifi2c_err wifi2c_keepalive(wifi2c_hnd hnd)
{
	return xchg(hnd, "k");
}

wifi2c_err wifi2c_reset(wifi2c_hnd hnd)
{
	return xchg(hnd, "e");
}

wifi2c_err wifi2c_start(wifi2c_hnd hnd)
{
	return xchg(hnd, "s");
}

wifi2c_err wifi2c_stop(wifi2c_hnd hnd)
{
	return xchg(hnd, "p");
}

wifi2c_err wifi2c_write(wifi2c_hnd hnd, uint8_t byte, bool *ack)
{
	int err;
	char buf[4];
	char rsp;

	snprintf(buf, sizeof(buf), "w%02x", byte);
	err = xchg(hnd, buf);
	if(err) {
		return err;
	}

	err = get(hnd, &rsp, 1);
	if(rsp != 'a' && rsp != 'n') {
		return WIFI2C_ERR_PROTOCOL;
	}

	if(ack) {
		*ack = rsp == 'a';
	}

	return WIFI2C_ERR_OK;
}

wifi2c_err wifi2c_read(wifi2c_hnd hnd, uint8_t *byte, bool ack)
{
	int err;
	char rsp[2];
	uint8_t hi, lo;

	err = xchg(hnd, ack ? "ra" : "rn");
	if(err) {
		return err;
	}

	err = get(hnd, rsp, 2);
	if(err) {
		return err;
	}

	if(!from_hex(&hi, rsp[0]) || !from_hex(&lo, rsp[1])) {
		return WIFI2C_ERR_PROTOCOL;
	}

	if(byte) {
		*byte = (hi << 4) | lo;
	}

	return WIFI2C_ERR_OK;
}
