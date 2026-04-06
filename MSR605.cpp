#include <iostream>
#include <stdio.h>
#include <signal.h>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "msr605.h"

#define DEVICE "/dev/ttyUSB0"

MSR605 *msr = new MSR605();

void sigproc(int x)
{ 		
		printf("Quitting...");
		msr->disconnect();
		delete(msr);
		exit(0);
}

void printTrack(const char *msg, unsigned char *buf, unsigned int len)
{
	printf("[%s]: ", msg);
	for(int x = 0; x < len; x++) printf("%02X", buf[x]);
	printf("\n");
}

void printTrackiso(const char *msg, unsigned char *buf, unsigned int len)
{
	printf("[%s]: ", msg);
	for(int x = 0; x < len; x++) printf("%c", buf[x]);
	printf("\n");
}

static bool is_hex(const char *hex)
{
	int len = strlen(hex);
	if (len == 0 || len % 2 != 0) return false;
	for (int i = 0; i < len; i++)
		if (!isxdigit((unsigned char)hex[i])) return false;
	return true;
}

static unsigned char *parse_hex(const char *hex, int *out_len)
{
	int len = strlen(hex) / 2;
	unsigned char *buf = (unsigned char *)malloc(len);
	const char *ptr = hex;
	for (int i = 0; i < len; i++, ptr += 2)
		sscanf(ptr, "%02hhx", &buf[i]);
	*out_len = len;
	return buf;
}

static void usage(const char *prog)
{
	printf("usage: %s -r|-w|-e|-i [--bulk] [--raw|--iso] [--hico|--loco] [--bpc <NNN>] [-1 <hex>] [-2 <hex>] [-3 <hex>] [--tracks <N...>] [-p <dev>]\n\n", prog);
	printf("  -r, --read            read card\n");
	printf("  -w, --write           write card (raw)\n");
	printf("  -e, --erase           erase card\n");
	printf("  -i, --info            retrieve device firmware and model info\n");
	printf("  -B, --bulk            write in bulk mode\n");
	printf("  -p, --port <dev>      serial device (default: /dev/ttyUSB0)\n");
	printf("  --raw / --iso         set data formatting (default: --raw)\n");
	printf("  --bpc <NNN>           bits per char for tracks 1/2/3 (default: 888)\n");
	printf("  -1, --track1 <hex>    track 1 data (write mode)\n");
	printf("  -2, --track2 <hex>    track 2 data (write mode)\n");
	printf("  -3, --track3 <hex>    track 3 data (write mode)\n");
	printf("  --tracks <N...>       tracks to erase, any combo of 1/2/3 (default: 123)\n");
	printf("  --hico / --loco       set coercivity (default: --hico)\n");
	printf("  -d, --debug           enable debug output\n");
}

int main(int argc, char *argv[])
{
	enum op_t   { OP_NONE, OP_READ, OP_WRITE, OP_ERASE, OP_INFO };
	enum mode_t { MODE_RAW = 1, MODE_ISO = 2 };
	enum        { OPT_RAW = 256, OPT_ISO, OPT_TRACKS, OPT_HICO, OPT_LOCO };

	int t1 = 8, t2 = 8, t3 = 8;
	op_t       op   = OP_NONE;
	mode_t     mode = MODE_RAW;
	char *device = DEVICE;
	const char *hex1 = NULL, *hex2 = NULL, *hex3 = NULL;
	bool erase1 = true, erase2 = true, erase3 = true;
	bool hico = true;
	bool bulk = false;
	bool debug = false;

	static struct option long_opts[] = {
		{"read",   no_argument,       NULL, 'r'},
		{"write",  no_argument,       NULL, 'w'},
		{"erase",  no_argument,       NULL, 'e'},
		{"info",   no_argument,       NULL, 'i'},
		{"bulk",   no_argument,       NULL, 'B'},
		{"port",   required_argument, NULL, 'p'},
		{"raw",    no_argument,       NULL, OPT_RAW},
		{"iso",    no_argument,       NULL, OPT_ISO},
		{"bpc",    required_argument, NULL, 'b'},
		{"track1", required_argument, NULL, '1'},
		{"track2", required_argument, NULL, '2'},
		{"track3", required_argument, NULL, '3'},
		{"tracks", required_argument, NULL, OPT_TRACKS},
		{"hico",   no_argument,       NULL, OPT_HICO},
		{"loco",   no_argument,       NULL, OPT_LOCO},
		{"debug",  no_argument,       NULL, 'd'},
		{NULL, 0, NULL, 0}
	};

	int opt;
	while ((opt = getopt_long(argc, argv, "rweiBdp:b:1:2:3:", long_opts, NULL)) != -1) {
		switch (opt) {
		case 'r': op = OP_READ;    break;
		case 'w': op = OP_WRITE;   break;
		case 'e': op = OP_ERASE;   break;
		case 'i': op = OP_INFO; break;
		case 'B': bulk = true;  break;
		case 'p': device = optarg;           break;
		case OPT_RAW:  mode = MODE_RAW; break;
		case OPT_ISO:  mode = MODE_ISO; break;
		case OPT_HICO:  hico = true;   break;
		case OPT_LOCO:  hico = false;  break;
		case 'd':       debug  = true;   break;
		case OPT_TRACKS:
			erase1 = erase2 = erase3 = false;
			for (const char *p = optarg; *p; p++) {
				if      (*p == '1') erase1 = true;
				else if (*p == '2') erase2 = true;
				else if (*p == '3') erase3 = true;
				else { printf("ERROR: --tracks accepts only digits 1, 2, 3\n"); return 1; }
			}
			break;
		case 'b':
			if (strlen(optarg) != 3) {
				printf("ERROR: --bpc requires exactly 3 digits (e.g. 755)\n");
				return 1;
			}
			t1 = optarg[0] - '0';
			t2 = optarg[1] - '0';
			t3 = optarg[2] - '0';
			break;
		case '1': hex1 = optarg; break;
		case '2': hex2 = optarg; break;
		case '3': hex3 = optarg; break;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	if (op == OP_NONE) {
		usage(argv[0]);
		return 1;
	}

	auto bad_bpc = [](int b){ return b < 5 || b > 8 || b == 6; };
	if (bad_bpc(t1)) { printf("ERROR: track1 bpc must be 5, 7, or 8\n"); return 1; }
	if (bad_bpc(t2)) { printf("ERROR: track2 bpc must be 5, 7, or 8\n"); return 1; }
	if (bad_bpc(t3)) { printf("ERROR: track3 bpc must be 5, 7, or 8\n"); return 1; }

	if (op == OP_WRITE && !hex1 && !hex2 && !hex3) {
		printf("ERROR: write requires at least one of --track1/-1, --track2/-2, --track3/-3\n");
		return 1;
	}
	if (bulk && op != OP_WRITE) {
		printf("ERROR: --bulk/-B only applies to -w/--write\n");
		return 1;
	}

	if (hex1 && !is_hex(hex1)) { printf("ERROR: -1/--track1 must be a valid hex string\n"); return 1; }
	if (hex2 && !is_hex(hex2)) { printf("ERROR: -2/--track2 must be a valid hex string\n"); return 1; }
	if (hex3 && !is_hex(hex3)) { printf("ERROR: -3/--track3 must be a valid hex string\n"); return 1; }

	if (debug) msr->setDebug(true);
	signal(SIGINT, sigproc);
	try {
		magnetic_stripe_t *data = NULL;
		msr->connect(device);
		printf("Connected to %s\n", device);
		msr->sendReset();
		msr->getFirmware();
		msr->getModel();
		msr->sendReset();

		msr->init();
		printf("Initialized MSR605.\n");

		if (op == OP_WRITE) {
			try {
				bool ok = hico ? msr->setHiCo() : msr->setLoCo();
				if (ok) printf("%s set successfully.\n", hico ? "HiCo" : "LoCo");
			} catch (const char *e) {
				printf("Error setting coercivity: %s\n", e);
			}

			printf("Writing in bulk mode... Press Ctrl+C to stop\n");
			int card = 0;
			do {
				msr->sendReset();

				int len1 = 0, len2 = 0, len3 = 0;
				data = (magnetic_stripe_t *) malloc(sizeof(magnetic_stripe_t));
				data->track1 = hex1 ? parse_hex(hex1, &len1) : NULL; data->t1_len = len1;
				data->track2 = hex2 ? parse_hex(hex2, &len2) : NULL; data->t2_len = len2;
				data->track3 = hex3 ? parse_hex(hex3, &len3) : NULL; data->t3_len = len3;

				if (bulk) printf("[card %d] ", ++card);
				printf("Waiting for swipe...\n");
				try {
					bool ok = msr->writeCard_raw(data, t1, t2, t3);
					if (ok) {
						if (bulk) printf("[card %d] Done.\n", card);
						else printf("Write successful.\n");
					}
				} catch (const char *e) {
					printf("Error writing card: %s\n", e);
				}

				free(data->track1); free(data->track2); free(data->track3);
			} while (bulk);
			free(data);
		} else if (op == OP_READ) {
			msr->setAllLEDOff();
			printf("Waiting for swipe...\n");
			try {
				if (mode == MODE_RAW) {
					data = msr->readCard_raw(t1, t2, t3);
					printf("Read successful.\n");
					printTrack("Track 1", data->track1, data->t1_len);
					printTrack("Track 2", data->track2, data->t2_len);
					printTrack("Track 3", data->track3, data->t3_len);
				} else {
					data = msr->readCard_iso(t1, t2, t3);
					printf("Read successful.\n");
					printTrackiso("Track 1", data->track1, data->t1_len);
					printTrackiso("Track 2", data->track2, data->t2_len);
					printTrackiso("Track 3", data->track3, data->t3_len);
				}
				msr->free_ms_data(data);
			} catch (const char *e) {
				printf("Error reading card: %s\n", e);
			}
		} else if (op == OP_ERASE) {
			printf("Waiting for swipe...\n");
			try {
				bool ok = msr->eraseCard(erase1, erase2, erase3);
				if (ok) printf("Erase successful.\n");
			} catch (const char *e) {
				printf("Error erasing card: %s\n", e);
			}
		} else if (op == OP_INFO) {
			msr->getFirmware();
			msr->getModel();
		}

		msr->disconnect();
	}
	catch (const char *msg) {
		printf("MSR605 Error: %s\n", msg);
	}

	delete(msr);
	return 0;
}
