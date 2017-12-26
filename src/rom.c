#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rom.h"


static char romname[16] = { '\0' };


bool loadrom(const uint8_t* const data)
{
	if (data == NULL)
		return false;

	memcpy(romname, &data[0x134], 15);
	printf("ROM NAME: %s\n", romname);
	return true;
}

void freerom(void)
{
	memset(romname, 0, 16);
}

