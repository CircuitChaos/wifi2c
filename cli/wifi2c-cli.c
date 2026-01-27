#include <wifi2c.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#if WIFI2C_VERSION != 1
#error Unsupported wifi2c library version
#endif

static bool call(const char *name, wifi2c_err err)
{
	if(err) {
		fprintf(stderr, "Error in wifi2c_%s(): %s\n", name, wifi2c_get_error_string(err));
		return false;
	}

	return true;
}

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

typedef enum {
	ST_IDLE,
	ST_READING,
	ST_WRITING,
} state_t;

static state_t state = ST_IDLE;

static void set_state(state_t new_state)
{
	if(new_state == state) {
		return;
	}

	if(state != ST_IDLE) {
		printf("\n");
	}

	switch(new_state) {
		case ST_READING:
			printf("Read: ");
			break;

		case ST_WRITING:
			printf("Write:");
			break;

		default:
			break;
	}

	state = new_state;
}

static void print(char cmd, uint8_t byte, bool ack)
{
	switch(cmd) {
		case 'o': /* Port opened */
			setvbuf(stdout, NULL, _IONBF, 0);
			printf("Connection established\n");
			break;

		case 'q': /* Terminating */
			set_state(ST_IDLE);
			printf("Terminating\n");
			break;

		case 'e':
			set_state(ST_IDLE);
			printf("Bus reset\n");
			break;

		case 's':
			set_state(ST_IDLE);
			printf("Start\n");
			break;

		case 'p':
			set_state(ST_IDLE);
			printf("Stop\n");
			break;

		case 'r':
		case 'w':
			set_state(cmd == 'r' ? ST_READING : ST_WRITING);
			printf(" %02x", byte);
			if(!ack) {
				printf("!");
			}

			break;

		case 'k':
			set_state(ST_IDLE);
			printf("Sent keepalive\n");
			break;

		case 'c':
			set_state(ST_IDLE);
			printf("Set clock to 0x%02x (approx. %u us)\n", byte, (unsigned) byte * 10);
			break;

		default:
			break;
	}
}

static void print2(char cmd)
{
	print(cmd, 0, false);
}

int main(int argc, char *const argv[])
{
	wifi2c_hnd hnd;
	int i;

	if(wifi2c_get_version() != WIFI2C_VERSION) {
		fprintf(stderr, "Header and library version mismatch\n");
		return EXIT_FAILURE;
	}

	if(argc < 3) {
		static const char text[] =
		    "Syntax: wifi2c-cli <host> <port> [<command> [<command>...]]\n"
		    "<port> is usually %s.\n"
		    "\n"
		    "List of supported commands, separated by space:\n"
		    "  k: send keepalive\n"
		    "  c<xx>: set clock (xx - clock in hex, lowercase)\n"
		    "  e: reset bus\n"
		    "  s: send start\n"
		    "  p: send stop\n"
		    "  ra: read byte, send ACK\n"
		    "  rn: read byte, send NAK\n"
		    "  w<xx>: write byte (xx - byte in hex, lowercase)\n"
		    "\n"
		    "Clock: delay after each change in 10 us steps.\n";

		printf(text, WIFI2C_DEFAULT_PORT);
		return EXIT_FAILURE;
	}

	if(!call("open", wifi2c_open(&hnd, argv[1], argv[2]))) {
		return EXIT_FAILURE;
	}

	print2('o');

	for(i = 3; i < argc; ++i) {
		bool rs;

		if(!strcmp(argv[i], "k")) {
			rs = call("keepalive", wifi2c_keepalive(hnd));
			if(rs) {
				print2('k');
			}
		}
		else if(!strcmp(argv[i], "e")) {
			rs = call("reset", wifi2c_reset(hnd));
			if(rs) {
				print2('e');
			}
		}
		else if(!strcmp(argv[i], "s")) {
			rs = call("start", wifi2c_start(hnd));
			if(rs) {
				print2('s');
			}
		}
		else if(!strcmp(argv[i], "p")) {
			rs = call("stop", wifi2c_stop(hnd));
			if(rs) {
				print2('p');
			}
		}
		else if(!strcmp(argv[i], "ra") || !strcmp(argv[i], "rn")) {
			uint8_t byte;
			const bool ack = !strcmp(argv[i], "ra");
			rs             = call("read", wifi2c_read(hnd, &byte, ack));
			if(rs) {
				print('r', byte, ack);
			}
		}
		else if(strlen(argv[i]) == 3 && (argv[i][0] == 'w' || argv[i][0] == 'c')) {
			uint8_t hi, lo;
			if(!from_hex(&hi, argv[i][1]) || !from_hex(&lo, argv[i][2])) {
				fprintf(stderr, "Can't decode hex in command %s\n", argv[i]);
				rs = false;
			}
			else if(argv[i][0] == 'w') {
				const uint8_t byte = (hi << 4) | lo;
				bool ack;
				rs = call("write", wifi2c_write(hnd, byte, &ack));
				if(rs) {
					print('w', byte, ack);
				}
			}
			else {
				const uint8_t byte = (hi << 4) | lo;
				rs                 = call("set_clock", wifi2c_set_clock(hnd, byte));
				if(rs) {
					print('c', byte, false);
				}
			}
		}
		else {
			fprintf(stderr, "Unknown command %s\n", argv[i]);
			rs = false;
		}

		if(!rs) {
			call("close", wifi2c_close(hnd));
			return EXIT_FAILURE;
		}
	}

	print2('q');
	return call("close", wifi2c_close(hnd)) ? EXIT_SUCCESS : EXIT_FAILURE;
}
