#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rom.h"


static char romname[16] = { '\0' };
static uint8_t cgbflag = 0;
static uint8_t sgbflag = 0;
static uint8_t romtype = 0;
static uint16_t rom_nbanks = 0;
static uint32_t romsize = 0;


bool loadrom(const uint8_t* const data)
{
	if (data == NULL)
		return false;

	memcpy(romname, &data[0x134], 15);
	cgbflag = data[0x143];
	sgbflag = data[0x146];
	romtype = data[0x147];

	const uint16_t banks1[9] = { 2, 4, 16, 32, 64, 128, 256, 512 };
	const uint8_t banks2[3] = { 72, 80, 96 };
	rom_nbanks = data[0x148] <= 0x08 ? banks1[data[0x148]]
	                                 : banks2[data[0x148] - 0x52];
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

