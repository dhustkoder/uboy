#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "rom.h"


static char romname[16] = { '\0' };
static uint8_t cgbflag = 0;
static uint8_t sgbflag = 0;
static uint8_t romtype = 0;
static uint16_t rom_nbanks = 0;
static uint32_t romsize = 0;
static uint8_t romdata[0x4000 * 512];


static bool readfile(const char* const filename)
{
	FILE* const file = fopen(filename, "r");
	if (file == NULL) {
		perror("Couldn't open file: ");
		return NULL;
	}

	int ret = true;

	fseek(file, 0, SEEK_END);
	const long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (fread(romdata, 1, size, file) < (unsigned long)size) {
		perror("Couldn't read file: ");
		ret = false;
	}

	fclose(file);
	return ret;
}

bool loadrom(const char* const filename)
{
	if (!readfile(filename))
		return false;

	memcpy(romname, &romdata[0x134], 15);
	cgbflag = romdata[0x143];
	sgbflag = romdata[0x146];
	romtype = romdata[0x147];

	const uint16_t banks1[9] = { 2, 4, 16, 32, 64, 128, 256, 512 };
	const uint8_t banks2[3] = { 72, 80, 96 };
	rom_nbanks = romdata[0x148] <= 0x08 ? banks1[romdata[0x148]]
	                                 : banks2[romdata[0x148] - 0x52];
	romsize = rom_nbanks * 0x4000;

	printf("Rom Name: %s\n"
	       "CGB Flag: $%X\n"
	       "SGB Flag: $%X\n"
	       "Rom Type: $%X\n"
	       "Rom N Banks: $%X\n"
	       "Rom Size: $%X\n",
	       romname, cgbflag,
	       sgbflag, romtype,
	       rom_nbanks, romsize);


	return true;
}

void freerom(void)
{
}

