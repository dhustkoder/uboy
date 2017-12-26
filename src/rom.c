#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rom.h"


static char romname[16] = { '\0' };
static uint8_t cgbflag = 0;

bool loadrom(const uint8_t* const data)
{
	if (data == NULL)
		return false;

	memcpy(romname, &data[0x134], 15);
	cgbflag = data[0x143];

	printf("ROM NAME: %s\n"
	       "CGB Flag: $%X\n",
	       romname, cgbflag);

	return true;
}

void freerom(void)
{
	memset(romname, 0, 16);
}

