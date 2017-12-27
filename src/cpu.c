#include "cpu.h"

extern uint16_t rom_nrombanks;
extern uint8_t rom_data[0x4000 * 512];

static uint16_t pc;


void resetcpu(void)
{
	pc = 0x0100;
}

uint8_t stepcpu(void)
{
	return 0;
}

