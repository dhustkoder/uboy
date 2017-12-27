#include <string.h>
#include "cpu.h"

extern uint16_t rom_nrombanks;
extern uint8_t rom_data[0x4000 * 512];


static struct { 
	uint8_t z : 1; 
	uint8_t n : 1; 
	uint8_t h : 1; 
	uint8_t c : 1; 
} fgs;

static struct {
	uint16_t pc;
	uint16_t sp;

	union {
		struct {
			uint8_t f, a;
		};
		uint16_t af;
	};

	union {
		struct {
			uint8_t c, b;
		};
		uint16_t bc;
	};

	union {
		struct {
			uint8_t e, d;
		};
		uint16_t de;
	};

	union {
		struct {
			uint8_t l, h;
		};
		uint16_t hl;
	};

} rgs;


void resetcpu(void)
{
	memset(&fgs, 0, sizeof fgs);
	memset(&rgs, 0, sizeof rgs);
	rgs.pc = 0x0100;
	rgs.sp = 0xFFFE;
	rgs.af = 0x01B0;
	rgs.bc = 0x0013;
	rgs.de = 0x00D8;
	rgs.hl = 0x014D;
}

uint8_t stepcpu(void)
{

}

