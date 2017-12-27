#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "rom.h"


uint16_t rom_nrombanks = 0;
uint8_t rom_data[0x4000 * 512];


static char romtitle[16] = { '\0' };
static uint8_t cgbflag = 0;
static uint8_t sgbflag = 0;
static uint8_t romtype = 0;


static bool readfile(const char* const filename)
{
	FILE* const file = fopen(filename, "r");
	if (file == NULL) {
		perror("Couldn't open file: ");
		return false;
	}

	int ret = true;

	fseek(file, 0, SEEK_END);
	const long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (fread(rom_data, 1, size, file) < (unsigned long)size) {
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

	memcpy(romtitle, &rom_data[0x134], 15);

	if (strlen(romtitle) == 0) {
		fprintf(stderr, "Couldn't read rom romtitle\n");
		return false;
	}

	cgbflag = rom_data[0x143];
	sgbflag = rom_data[0x146];
	romtype = rom_data[0x147];

	const uint16_t nbanks1[9] = { 2, 4, 16, 32, 64, 128, 256, 512 };
	const uint8_t nbanks2[3] = { 72, 80, 96 };

	if (rom_data[0x148] <= 0x08) {
		rom_nrombanks = nbanks1[rom_data[0x148]];
	} else if (rom_data[0x148] >= 0x52 && rom_data[0x148] <= 0x54) {
		rom_nrombanks = nbanks2[rom_data[0x148] - 0x52];
	} else {
		fprintf(stderr, "Couldn't read number of banks\n");
		return false;
	}

	printf("Rom Title: %s\n"
	       "CGB Flag: $%X\n"
	       "SGB Flag: $%X\n"
	       "Rom Type: $%X\n"
	       "Rom Number of ROM Banks: $%X\n"
	       "Rom Size: $%X\n",
	       romtitle, cgbflag,
	       sgbflag, romtype,
	       rom_nrombanks, rom_nrombanks * 0x4000 * 0x10);


	return true;
}

void freerom(void)
{
}

